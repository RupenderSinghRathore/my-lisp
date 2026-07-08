#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	mpc_parser_t *number;
	mpc_parser_t *operator;
	mpc_parser_t *expr;
	mpc_parser_t *my_lisp;
} Grammer;

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

	mpc_err_t *err = mpca_lang(MPCA_LANG_DEFAULT, "    \
    number   : /-?[0-9]+/ ;                            \
    operator : '+' | '-' | '*' | '/' ;                 \
    expr     : <number> | '(' <operator> <expr>+ ')' ; \
    my_lisp    : /^/ <operator> <expr>+ /$/ ;          \
    ",
	                           g->number, g->operator, g->expr, g->my_lisp);
	assert(!err);

	return g;
}

int number_of_nodes(mpc_ast_t *tree) {
	if (tree == NULL) {
		return 0;
	} else if (tree->children_num == 0) {
		return 1;
	}
	int total = 1;
	for (int i = 0; i < tree->children_num; i++) {
		total += number_of_nodes(tree->children[i]);
	}
	return total;
}
long int evaluate_tree(mpc_ast_t *tree) {
	if (tree == NULL)
		return 0;

	if (strstr(tree->tag, "number"))
		return atoi(tree->contents);

	if (tree->children_num == 0)
		return 0;

	long int final = 0;

	// TODO: polish notations algorithm

	return final;
}

int main(void) {

	Grammer *grammer = create_lisp_grammer();

	puts("my-lisp Version 0.001");

	const char prompt[] = "lisp> ";
	bool quit = false;
	while (true) {
		char *res = readline(prompt);
		assert(res);

		if (!strcmp(res, "")) {
			goto cleanup;
		} else if (!strcmp(res, "exit")) {
			quit = true;
			goto cleanup;
		}

		mpc_result_t r;
		if (mpc_parse("<stdin>", res, grammer->my_lisp, &r)) {
			mpc_ast_print(r.output);
			printf("no. of nodes: %d\n", number_of_nodes(r.output));
			mpc_ast_delete(r.output);
			add_history(res);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

	cleanup:
		free(res);
		if (quit)
			break;
	}

	clean_grammer(grammer);
	return 0;
}
