#include "grammer.h"
#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int main(void) {

	Grammer *grammer = create_lisp_grammer();

	puts("my-lisp Version 0.001");

	const char prompt[] = "lisp> ";
	bool quit = false;
	while (true) {
		char *res = readline(prompt);
		assert(res);

		if (strcmp(res, "") == 0)
			goto cleanup;

		if (strcmp(res, "exit") == 0) {
			quit = true;
			goto cleanup;
		}

		mpc_result_t r;
		if (mpc_parse("<stdin>", res, grammer->my_lisp, &r)) {
			// mpc_ast_print(r.output);
			lval_print_ln(eval(r.output));
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
