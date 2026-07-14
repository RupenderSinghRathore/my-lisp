#include "my_lisp.h"
#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <stdbool.h>
#include <stdlib.h>

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
