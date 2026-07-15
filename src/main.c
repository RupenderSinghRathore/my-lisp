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
    list *f_map = new_func_list();
    add_builtin_funcs(f_map);

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
            // lval_type_print(l->type);
            l = eval(f_map, l);
            lval_print_ln(l);

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
    func_list_del(f_map);
    return 0;
}
