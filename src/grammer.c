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

void del_lval(lval *l) {
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
            del_lval(l->cell[i]);
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

// lval eval_op(char *op, lval a, lval b) {
//   if (a.type == LVAL_ERR)
//     return a;
//   if (b.type == LVAL_ERR)
//     return b;
//
//   double x = a.num;
//   double y = b.num;
//
//   if (strcmp(op, "+") == 0) return new_lval_num(x + y);
//
//   if (strcmp(op, "-") == 0) return new_lval_num(x - y);
//
//   if (strcmp(op, "*") == 0) return new_lval_num(x * y);
//
//   if (strcmp(op, "/") == 0) return (y == 0) ? new_lval_err(LERR_DIV_BY_ZERO)
//   : new_lval_num(x / y);
//
//   if (strcmp(op, "%") == 0)
//     return (y == 0) ? new_lval_err(LERR_DIV_BY_ZERO)
//                     : new_lval_num(fmod(x, y));
//
//   if (strcmp(op, "^") == 0) return new_lval_num(pow(x, y));
//
//   if (strcmp(op, "max") == 0)
//     return new_lval_num(x > y ? x : y);
//
//   if (strcmp(op, "min") == 0)
//     return new_lval_num(x < y ? x : y);
//
//   return new_lval_err(LERR_BAD_OP);
// }
//
// lval eval(mpc_ast_t *tree) {
//
//   if (strstr(tree->tag, "number")) {
//     errno = 0;
//     double num = strtod(tree->contents, NULL);
//     return (errno == ERANGE) ? new_lval_err(LERR_BAD_NUM)
//                              : new_lval_num(num);
//   }
//
//   char *op = tree->children[1]->contents;
//   lval x = eval(tree->children[2]);
//
//   int i = 3;
//
//   if (strcmp(op, "-") == 0 && i + 1 >= tree->children_num)
//     x.num = -x.num;
//
//   while (strstr(tree->children[i]->tag, "expr")) {
//     x = eval_op(op, x, eval(tree->children[i]));
//     i++;
//   }
//
//   return x;
// }

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
