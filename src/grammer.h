#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <stdbool.h>

typedef struct {
	mpc_parser_t *number;
	mpc_parser_t *operator;
	mpc_parser_t *expr;
	mpc_parser_t *my_lisp;
} Grammer;

void clean_grammer(Grammer *g);

Grammer *create_lisp_grammer(void);

typedef enum { LVAL_NUM,
	           LVAL_ERR } lval_type;

typedef enum { LERR_DIV_BY_ZERO,
	           LERR_BAD_OP,
	           LERR_BAD_NUM } lval_err;

typedef struct {
	lval_type type;
	union {
		long num;
		lval_err err;
	};
} lval;

lval new_lval_num(long num);
lval new_lval_err(lval_err err);

void lval_print(lval l);
void lval_print_ln(lval l);

int number_of_leaf(mpc_ast_t *tree);
int number_of_branches(mpc_ast_t *tree);
lval eval_op(char *op, lval a, lval b);

lval eval(mpc_ast_t *tree);
