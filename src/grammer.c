#include "grammer.h"
#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clean_grammer(Grammer *g) {
    mpc_cleanup(5, g->number, g->symbol, g->sexpr, g->expr, g->my_lisp);
    free(g);
}

Grammer *create_lisp_grammer(void) {
    Grammer *g;
    g = malloc(sizeof(*g));

    g->number = mpc_new("number");
    g->symbol = mpc_new("symbol");
    g->sexpr = mpc_new("sexpr");
    g->expr = mpc_new("expr");
    g->my_lisp = mpc_new("my_lisp");

    mpc_err_t *err =
        mpca_lang(MPCA_LANG_DEFAULT, "    \
    number   : /-?([0-9]+(\\.[0-9]*)?|\\.[0-9]+)/ ;    \
    symbol : '+' | '-' | '*' | '/' | '%' | '^' | \"max\" | \"min\" ;     \
    sexpr     : '(' <expr>* ')' ;                      \
    expr     : <number> | <symbol> | <sexpr> ; \
    my_lisp    : /^/ <expr>* /$/ ;          \
    ",
                  g->number, g->symbol, g->sexpr, g->expr, g->my_lisp);
    assert(!err);
    // mpc_err_print(err);

    return g;
}

lval *new_lval_num(double num) {
    lval *l;
    l = malloc(sizeof(*l));
    l->num = num;
    l->type = LVAL_NUM;
    return l;
}
lval *new_lval_err(char *e) {
    lval *l;
    l = malloc(sizeof(*l));

    l->err = malloc(strlen(e) + 1);
    strcpy(l->err, e);

    l->type = LVAL_ERR;
    return l;
}
lval *new_lval_sym(char *s) {
    lval *l;
    l = malloc(sizeof(*l));

    l->sym = malloc(strlen(s) + 1);
    strcpy(l->sym, s);

    l->type = LVAL_SYM;
    return l;
}
lval *new_lval_sexpr(void) {
    lval *l;
    l = malloc(sizeof(*l));
    l->count = 0;
    l->type = LVAL_SEXPR;
    l->cell = NULL;
    return l;
}

void lval_del(lval *l) {
    switch (l->type) {
    case LVAL_NUM:
        break;
    case LVAL_ERR:
        free(l->err);
        break;
    case LVAL_SYM:
        free(l->sym);
        break;
    case LVAL_SEXPR:
        for (int i = 0; i < l->count; i++)
            lval_del(l->cell[i]);
        free(l->cell);
        break;
    }
    free(l);
}

lval *lval_add(lval *l, lval *c) {
    l->count++;
    l->cell = realloc(l->cell, sizeof(void *) * l->count);
    l->cell[l->count - 1] = c;
    return l;
}

void lval_print_sexpr(lval *l, char start, char end) {
    putchar(start);
    for (int i = 0; i < l->count; i++) {
        lval_print(l->cell[i]);
        if (i != l->count - 1)
            putchar(' ');
    }
    putchar(end);
}

void lval_print(lval *l) {
    switch (l->type) {
    case LVAL_NUM:
        printf("%g", l->num);
        break;

    case LVAL_ERR:
        printf("Error: %s", l->err);
        break;

    case LVAL_SYM:
        printf("%s", l->sym);
        break;

    case LVAL_SEXPR:
        lval_print_sexpr(l, '(', ')');
        break;
    }
}
void lval_print_ln(lval *l) {
    lval_print(l);
    putchar('\n');
}

lval *eval_sexpr(lval *v) {
    lval *result = NULL;
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = eval(v->cell[i]);

        if (v->cell[i]->type == LVAL_ERR) {
            result = new_lval_err(v->cell[i]->err);
            goto cleanup;
        }
    }
    if (v->count == 0)
        return v;

    lval *x = v->cell[0];
    if (x->type != LVAL_SYM) {
        result = new_lval_err("s-expression does not start with a symbol!");
        goto cleanup;
    }

    result = builtin_op(v, x->sym);

cleanup:
    lval_del(v);
    return result;
}

lval *builtin_op(lval *v, char *op) {
    for (int i = 1; i < v->count; i++)
        if (v->cell[i]->type != LVAL_NUM)
            return new_lval_err("operands must be a number!");

    if (v->count < 2)
        return new_lval_err("not enough operands");

    double x = v->cell[1]->num;

    int i = 2;
    // nagation
    if (strcmp(op, "-") == 0 && i == v->count)
        x = -x;

    for (; i < v->count; i++) {
        double y = v->cell[i]->num;

        if (strcmp(op, "+") == 0)
            x = x + y;

        else if (strcmp(op, "-") == 0)
            x = x - y;

        else if (strcmp(op, "*") == 0)
            x = x * y;

        else if (strcmp(op, "/") == 0) {
            if (y == 0)
                return new_lval_err("can't divide by zero!");
            x = x / y;
        } else if (strcmp(op, "%") == 0) {
            if (y == 0)
                return new_lval_err("can't divide by zero!");
            x = fmod(x, y);
        } else if (strcmp(op, "^") == 0)
            x = pow(x, y);

        else if (strcmp(op, "max") == 0)
            x = x > y ? x : y;

        else if (strcmp(op, "min") == 0)
            x = x < y ? x : y;

        else
            return new_lval_err("unknown symbol");
    }
    return new_lval_num(x);
}

lval *eval(lval *v) {
    if (v->type == LVAL_SEXPR) {
        return eval_sexpr(v);
    }
    return v;
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
        return new_lval_sym(t->contents);

    lval *x = NULL;
    if (strstr(t->tag, ">"))
        x = new_lval_sexpr();
    else if (strstr(t->tag, "sexpr"))
        x = new_lval_sexpr();

    for (int i = 0; i < t->children_num; i++) {
        if ((strcmp(t->children[i]->contents, "(") == 0) ||
            (strcmp(t->children[i]->contents, ")") == 0) ||
            (strcmp(t->children[i]->tag, "regex") == 0))
            continue;

        x = lval_add(x, lval_read(t->children[i]));
    }
    if (x == NULL) {
        x = new_lval_sexpr();
    }
    return x;
}
