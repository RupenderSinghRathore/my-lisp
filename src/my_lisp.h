#include "list.h"
#include "mpc.h"
#include <assert.h>
#include <editline/readline.h>
#include <stdbool.h>

typedef struct {
    mpc_parser_t *number;
    mpc_parser_t *symbol;
    mpc_parser_t *sexpr;
    mpc_parser_t *qexpr;
    mpc_parser_t *expr;
    mpc_parser_t *my_lisp;
} Grammer;

void clean_grammer(Grammer *g);
Grammer *create_lisp_grammer(void);

typedef enum {
    LVAL_NUM,
    LVAL_SYM,
    LVAL_FUNC,
    LVAL_SEXPR,
    LVAL_QEXPR,
    LVAL_ERR
} lval_type;

void lval_type_print(lval_type t);

typedef enum { LERR_DIV_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM } lval_err;

typedef struct lval lval;

typedef lval *(*builtin_f)(list *f, list *operands);
typedef struct {
    char *sym;
    lval *val;
} func_map;

list *new_env_list(void);
void env_list_del(list *l);
void env_del(void *v);
void add_builtin_funcs(list *l);
void env_add(list *l, char *sym, lval *f);

lval *env_mapper(list *f, lval *v);

struct lval {
    lval_type type;

    union {
        double num;
        char *err;
        char *sym;
        builtin_f func;
        list *cell;
    };
};

lval *new_lval_num(double num);
lval *new_lval_err(const char *err);
lval *new_lval_func(builtin_f f);
lval *new_lval_symbol(char *sym);
lval *new_lval_sexpr(void);
lval *new_lval_qexpr(void);

void *lval_clone(void *v);
void lval_del(void *v);

void lval_print(lval *v);
void lval_print_ln(lval *v);
void lval_print_expr(lval *v, char start, char end);

lval *eval(list *f, lval *v);
lval *lval_read(mpc_ast_t *tree);
lval *builtin_op(lval *v, func_map *s);
