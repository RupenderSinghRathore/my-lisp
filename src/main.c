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

long eval_op(char *op, long a, long b) {
	if (strcmp(op, "+") == 0) {
		return a + b;
	} else if (strcmp(op, "-") == 0) {
		return a - b;
	} else if (strcmp(op, "*") == 0) {
		return a * b;
	} else if (strcmp(op, "/") == 0) {
		return a / b;
	}
	return 0;
}

long int eval(mpc_ast_t *tree) {
	if (strstr(tree->tag, "number"))
		return atoi(tree->contents);

	char *op = tree->children[1]->contents;
	long int x = eval(tree->children[2]);

	int i = 3;
	while (strstr(tree->children[i]->tag, "expr")) {
		x = eval_op(op, x, eval(tree->children[i]));
		i++;
	}

	return x;
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
			// mpc_ast_print(r.output);
			printf("=> %ld\n", eval(r.output));
			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		add_history(res);

	cleanup:
		free(res);
		if (quit)
			break;
	}

	clean_grammer(grammer);
	return 0;
}
