#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <stdbool.h>

typedef struct {
    mpc_parser_t *number;
    mpc_parser_t *symbol;
    mpc_parser_t *sexpr;
    mpc_parser_t *expr;
    mpc_parser_t *my_lisp;
} Grammer;

void clean_grammer(Grammer *g);

Grammer *create_lisp_grammer(void);

typedef enum { LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_ERR } lval_type;

typedef enum { LERR_DIV_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM } lval_err;

typedef struct lval {
    lval_type type;
    int count;

    union {
        double num;
        char *err;
        char *sym;
        struct lval **cell;
    };

} lval;

lval *new_lval_num(double num);
lval *new_lval_err(char *err);
lval *new_lval_sym(char *sym);
lval *new_lval_sexpr(void);

lval *lval_add(lval *l, lval *c);

void del_lval(lval *l);

void lval_print(lval *l);
void lval_print_ln(lval *l);
void lval_print_sexpr(lval *l, char start, char end);

int number_of_leaf(mpc_ast_t *tree);
int number_of_branches(mpc_ast_t *tree);
lval eval_op(char *op, lval a, lval b);

lval eval(mpc_ast_t *tree);
lval *lval_read(mpc_ast_t *tree);
