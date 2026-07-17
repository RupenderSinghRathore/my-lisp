#include "mpc.h"
#include "my_lisp.h"
#include <assert.h>
#include <editline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {

    Grammer *grammer = create_lisp_grammer();
    list *env = new_env_list();
    add_builtin_funcs(env);

    puts("my-lisp Version 0.001");

    const char prompt[] = "lisp> ";
    bool quit = false;
    while (!quit) {
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
            lval *l = lval_read(r.output);
            // printf("%s\n", lval_type(t));
            // lval_type_print(l->type);
            l = eval(env, l);
            lval_println(l);

            lval_del(l);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        add_history(res);

    cleanup:
        free(res);
    }

    clean_grammer(grammer);
    env_list_del(env);
    return 0;
}
