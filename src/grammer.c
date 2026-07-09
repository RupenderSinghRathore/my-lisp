#include "grammer.h"
#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

void clean_grammer(Grammer *g) {
	mpc_cleanup(4, g->number, g->operator, g->expr, g->my_lisp);
	free(g);
}

Grammer *create_lisp_grammer(void) {
	Grammer *g;
	g = malloc(sizeof(*g));

	g->number = mpc_new("number");
	g->operator = mpc_new("operator");
	g->expr = mpc_new("expr");
	g->my_lisp = mpc_new("my_lisp");

	/*
	 * // TODO: maybe switch to double
	 * // number   : /-?[0-9]+/ ; \
	 */

	mpc_err_t *err = mpca_lang(MPCA_LANG_DEFAULT, "    \
	number   : /-?([0-9]+(\\.[0-9]*)?|\\.[0-9]+)/ ;    \
    operator : '+' | '-' | '*' | '/' | '%' | '^' | \"max\" | \"min\" ;     \
    expr     : <number> | '(' <operator> <expr>+ ')' ; \
    my_lisp    : /^/ <operator> <expr>+ /$/ ;          \
    ",
	                           g->number, g->operator, g->expr, g->my_lisp);
	assert(!err);
	// mpc_err_print(err);

	return g;
}

lval new_lval_num(double num) {
	return (lval){
	    .type = LVAL_NUM,
	    .num = num,
	};
}
lval new_lval_err(lval_err err) {
	return (lval){
	    .type = LVAL_ERR,
	    .err = err,
	};
}

void lval_print(lval l) {
	switch (l.type) {
	case LVAL_NUM:
		printf("=> %g", l.num);
		break;
	case LVAL_ERR:
		switch (l.err) {
		case LERR_DIV_BY_ZERO:
			printf("Error: Division By Zero!");
			break;
		case LERR_BAD_OP:
			printf("Error: Invalid Operator!");
			break;
		case LERR_BAD_NUM:
			printf("Error: Invalid Number!");
			break;
		}
		break;
	}
}
void lval_print_ln(lval l) {
	lval_print(l);
	putchar('\n');
}

int number_of_leaf(mpc_ast_t *tree) {
	if (tree->children_num == 0) {
		return 1;
	}
	int count = 0;
	for (int i = 0; i < tree->children_num; i++) {
		count += number_of_leaf(tree->children[i]);
	}
	return count;
}
int number_of_branches(mpc_ast_t *tree) {
	if (tree->children_num == 0) {
		return 0;
	}
	int count = 0;
	for (int i = 0; i < tree->children_num; i++) {
		count += number_of_branches(tree->children[i]);
		count++;
	}
	return count;
}

lval eval_op(char *op, lval a, lval b) {
	if (a.type == LVAL_ERR)
		return a;
	if (b.type == LVAL_ERR)
		return b;

	double x = a.num;
	double y = b.num;

	if (strcmp(op, "+") == 0)
		return new_lval_num(x + y);

	if (strcmp(op, "-") == 0)
		return new_lval_num(x - y);

	if (strcmp(op, "*") == 0)
		return new_lval_num(x * y);

	if (strcmp(op, "/") == 0)
		return (y == 0) ? new_lval_err(LERR_DIV_BY_ZERO) : new_lval_num(x / y);

	if (strcmp(op, "%") == 0)
		return (y == 0) ? new_lval_err(LERR_DIV_BY_ZERO) : new_lval_num(fmod(x, y));

	if (strcmp(op, "^") == 0)
		return new_lval_num(pow(x, y));

	if (strcmp(op, "max") == 0)
		return new_lval_num(x > y ? x : y);

	if (strcmp(op, "min") == 0)
		return new_lval_num(x < y ? x : y);

	return new_lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *tree) {

	if (strstr(tree->tag, "number")) {
		errno = 0;
		double num = strtod(tree->contents, NULL);
		return (errno == ERANGE) ? new_lval_err(LERR_BAD_NUM) : new_lval_num(num);
	}

	char *op = tree->children[1]->contents;
	lval x = eval(tree->children[2]);

	int i = 3;

	if (strcmp(op, "-") == 0 && i + 1 >= tree->children_num)
		x.num = -x.num;

	while (strstr(tree->children[i]->tag, "expr")) {
		x = eval_op(op, x, eval(tree->children[i]));
		i++;
	}

	return x;
}
