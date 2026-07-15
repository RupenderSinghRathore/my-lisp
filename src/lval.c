#include "mpc.h"
#include "my_lisp.h"
#include <assert.h>
#include <editline/readline.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lval_type_print(lval_type t) {
    switch (t) {
    case LVAL_NUM:
        printf("lval_num");
        break;
    case LVAL_SYM:
        printf("lval_sym");
        break;
    case LVAL_SEXPR:
        printf("lval_sexpr");
        break;
    case LVAL_FUNC:
        printf("lval_func");
        break;
    case LVAL_QEXPR:
        printf("lval_qexpr");
        break;
    case LVAL_ERR:
        printf("lval_err");
        break;
    }
    putchar('\n');
}

lval *new_lval_num(double num) {
    lval *v = malloc(sizeof(*v));
    v->num = num;
    v->type = LVAL_NUM;
    return v;
}
lval *new_lval_err(const char *e) {
    lval *v = malloc(sizeof(*v));

    v->err = malloc(strlen(e) + 1);
    strcpy(v->err, e);

    v->type = LVAL_ERR;
    return v;
}
lval *new_lval_symbol(char *s) {
    lval *v = malloc(sizeof(*v));

    v->sym = strdup(s);
    v->type = LVAL_SYM;
    return v;
}
lval *new_lval_func(builtin_f f) {
    lval *v = malloc(sizeof(*v));
    v->type = LVAL_FUNC;
    v->func = f;
    return v;
}

lval *new_lval_sexpr(void) {
    lval *v = malloc(sizeof(*v));
    v->type = LVAL_SEXPR;
    v->cell = new_list();
    return v;
}
lval *new_lval_qexpr(void) {
    lval *v = malloc(sizeof(*v));
    v->type = LVAL_QEXPR;
    v->cell = new_list();
    return v;
}

void lval_del(void *g) {
    lval *v = g;
    if (!v)
        return;
    switch (v->type) {
    case LVAL_NUM:
        break;
    case LVAL_ERR:
        free(v->err);
        break;
    case LVAL_SYM:
        free(v->sym);
        break;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        list_del(v->cell, lval_del);
        break;
    case LVAL_FUNC:
        break;
    }
    free(v);
}

void *lval_clone(void *g) {
    lval *v = g;
    switch (v->type) {
    case LVAL_NUM:
        return new_lval_num(v->num);
    case LVAL_ERR:
        return new_lval_err(v->err);
    case LVAL_SYM:
        return new_lval_symbol(v->sym);
    case LVAL_FUNC:
        return new_lval_func(v->func);

    case LVAL_SEXPR:
    case LVAL_QEXPR: {
        lval *x = new_lval_qexpr();
        x->type = v->type;
        x->cell = list_clone(v->cell, lval_clone);
        return x;
    }
    }
    return NULL;
}

void lval_print_expr(lval *v, char start, char end) {
    putchar(start);
    for (int i = 0; i < v->cell->len; i++) {
        lval_print(v->cell->arr[i]);
        if (i != v->cell->len - 1)
            putchar(' ');
    }
    putchar(end);
}

void lval_print(lval *v) {
    switch (v->type) {
    case LVAL_NUM:
        printf("%g", v->num);
        break;

    case LVAL_ERR:
        printf("Error: %s", v->err);
        break;

    case LVAL_SYM:
        printf("%s", v->sym);
        break;

    case LVAL_FUNC:
        printf("<func>");
        break;

    case LVAL_SEXPR:
        lval_print_expr(v, '(', ')');
        break;
    case LVAL_QEXPR:
        lval_print_expr(v, '{', '}');
        break;
    }
}
void lval_print_ln(lval *v) {
    lval_print(v);
    putchar('\n');
}

lval *eval_sexpr(list *f, lval *v) {
    lval *result = NULL;
    lval *x = NULL;

    list *cell = v->cell;
    for (int i = 0; i < cell->len; i++) {
        cell->arr[i] = eval(f, cell->arr[i]);

        if (((lval *)cell->arr[i])->type == LVAL_ERR) {
            result = list_take(cell, i);
            goto cleanup;
        }
    }
    if (cell->len == 0)
        return v;
    else if (cell->len == 1) {
        result = list_take(cell, 0);
        goto cleanup;
    }

    x = list_take(cell, 0);
    if (x->type != LVAL_FUNC) {
        result = new_lval_err("s-expression does not start with a function!");
        goto cleanup;
    }
    result = x->func(f, cell);

cleanup:
    lval_del(x);
    lval_del(v);
    return result;
}

lval *eval(list *f, lval *v) {
    switch (v->type) {
    case LVAL_SYM: {
        lval *e = ops_mapper(f, v);
        lval_del(v);
        return e;
    }
    case LVAL_SEXPR:
        return eval_sexpr(f, v);
    default:
        return v;
    }
}

lval *lval_read_number(mpc_ast_t *t) {
    errno = 0;
    double num = strtod(t->contents, NULL);
    return (errno == ERANGE) ? new_lval_err("invalid number")
                             : new_lval_num(num);
}

lval *lval_read(mpc_ast_t *t) {
    if (strstr(t->tag, "number"))
        return lval_read_number(t);
    if (strstr(t->tag, "symbol"))
        return new_lval_symbol(t->contents);

    lval *x = NULL;
    if (strstr(t->tag, "sexpr"))
        x = new_lval_sexpr();
    else if (strstr(t->tag, "qexpr"))
        x = new_lval_qexpr();
    else if (strstr(t->tag, ">"))
        x = new_lval_sexpr();

    for (int i = 0; i < t->children_num; i++) {
        if ((strcmp(t->children[i]->contents, "(") == 0) ||
            (strcmp(t->children[i]->contents, ")") == 0) ||
            (strcmp(t->children[i]->contents, "{") == 0) ||
            (strcmp(t->children[i]->contents, "}") == 0) ||
            (strcmp(t->children[i]->tag, "regex") == 0))
            continue;

        list_push(x->cell, lval_read(t->children[i]));
    }

    return x;
}
