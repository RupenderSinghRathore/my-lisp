#include "grammer.h"
#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clean_grammer(Grammer *g) {
    mpc_cleanup(6, g->number, g->symbol, g->sexpr, g->qexpr, g->expr,
                g->my_lisp);
    free(g);
}

Grammer *create_lisp_grammer(void) {
    Grammer *g = malloc(sizeof(*g));

    g->number = mpc_new("number");
    g->symbol = mpc_new("symbol");
    g->sexpr = mpc_new("sexpr");
    g->qexpr = mpc_new("qexpr");
    g->expr = mpc_new("expr");
    g->my_lisp = mpc_new("my_lisp");

    mpc_err_t *err = mpca_lang(MPCA_LANG_DEFAULT, "                     \
    number   : /-?([0-9]+(\\.[0-9]*)?|\\.[0-9]+)/ ;                     \
    symbol : '+' | '-' | '*' | '/' | '%' | '^' | \"max\" | \"min\"      \
           | \"list\" | \"head\" | \"tail\" | \"join\" | \"eval\" ;     \
    sexpr     : '(' <expr>* ')' ;                                       \
    qexpr     : '{' <expr>* '}' ;                                       \
    expr     : <number> | <symbol> | <sexpr> | <qexpr> ;                \
    my_lisp    : /^/ <expr>* /$/ ;                                      \
    ",
                               g->number, g->symbol, g->sexpr, g->qexpr,
                               g->expr, g->my_lisp);
    assert(!err);
    // mpc_err_print(err);

    return g;
}

void lval_type_print(lval_type t) {
    switch (t) {
    case LVAL_NUM:
        printf("lval_num\n");
        break;
    case LVAL_SYM:
        printf("lval_sym\n");
        break;
    case LVAL_SEXPR:
        printf("lval_sexpr\n");
        break;
    case LVAL_QEXPR:
        printf("lval_qexpr\n");
        break;
    case LVAL_ERR:
        printf("lval_err\n");
        break;
    }
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
lval *new_lval_op(char *s) {
    lval *v = malloc(sizeof(*v));

    v->op = ops_mapper(s);
    v->type = LVAL_SYM;
    return v;
}

lval *new_lval_sexpr(void) {
    lval *v = malloc(sizeof(*v));
    v->count = 0;
    v->type = LVAL_SEXPR;
    v->cell = NULL;
    return v;
}
lval *new_lval_qexpr(void) {
    lval *v = malloc(sizeof(*v));
    v->count = 0;
    v->type = LVAL_QEXPR;
    v->cell = NULL;
    return v;
}

void lval_del(lval *v) {
    switch (v->type) {
    case LVAL_NUM:
        break;
    case LVAL_ERR:
        free(v->err);
        break;
    case LVAL_SYM:
        free(v->op->sym);
        free(v->op);
        break;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        for (int i = 0; i < v->count; i++)
            lval_del(v->cell[i]);
        free(v->cell);
        break;
    }
    free(v);
}

lval *lval_clone(lval *v) {
    switch (v->type) {
    case LVAL_NUM:
        return new_lval_num(v->num);
    case LVAL_ERR:
        return new_lval_err(v->err);
    case LVAL_SYM:
        return new_lval_op(v->op->sym);

    case LVAL_SEXPR:
    case LVAL_QEXPR: {
        lval *x = new_lval_qexpr();
        x->type = v->type;
        for (int i = 0; i < v->count; i++)
            lval_add(x, lval_clone(v->cell[i]));
        return x;
    }
    }
    return NULL;
}

lval *lval_add(lval *v, lval *c) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(void *) * v->count);
    v->cell[v->count - 1] = c;
    return v;
}

void lval_print_expr(lval *v, char start, char end) {
    putchar(start);
    for (int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);
        if (i != v->count - 1)
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
        printf("%s", v->op->sym);
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

lval *eval_sexpr(lval *v) {
    lval *result = NULL;
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = eval(v->cell[i]);

        if (v->cell[i]->type == LVAL_ERR) {
            result = lval_clone(v->cell[i]);
            goto cleanup;
        }
    }
    if (v->count == 0)
        return v;
    else if (v->count == 1) {
        result = lval_clone(v->cell[0]);
        goto cleanup;
    }

    lval *x = v->cell[0];
    if (x->type != LVAL_SYM) {
        result = new_lval_err("s-expression does not start with a symbol!");
        goto cleanup;
    }

    result = builtin_op(v, x->op);

cleanup:
    lval_del(v);
    return result;
}
lval *eval_qexpr(lval *v) {
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = eval(v->cell[i]);

        if (v->cell[i]->type == LVAL_ERR) {
            lval *e = lval_clone(v->cell[i]);
            lval_del(v);
            return e;
        }
    }
    return v;
}

lval *builtin_op(lval *v, Operator *op) {
    if (v->count < 2)
        return new_lval_err("not enough operands");

    for (int i = 1; i < v->count; i++)
        if (v->cell[i]->type != LVAL_NUM)
            return new_lval_err("operands must be a number!");

    lval **operands = v->cell + 1;
    return op->eval(operands, v->count-1);
}

lval *eval(lval *v) {
    if (v->type == LVAL_SEXPR)
        return eval_sexpr(v);
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
        return new_lval_op(t->contents);

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

        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}
