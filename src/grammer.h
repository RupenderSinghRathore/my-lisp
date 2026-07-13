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

typedef enum { LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_ERR } lval_type;

void lval_type_print(lval_type t);

typedef enum { LERR_DIV_BY_ZERO, LERR_BAD_OP, LERR_BAD_NUM } lval_err;

typedef struct lval lval;

typedef lval *(*eval_op)(lval **operands, int n);
typedef struct {
    char *sym;
    eval_op eval;
} Operator;

struct lval {
    lval_type type;
    int count;

    union {
        double num;
        char *err;
        Operator *op;
        struct lval **cell;
    };
};

lval *new_lval_num(double num);
lval *new_lval_err(const char *err);
lval *new_lval_op(char *sym);
lval *new_lval_expr(void);
lval *new_lval_sexpr(void);
lval *new_lval_qexpr(void);

lval *lval_add(lval *v, lval *c);
lval *lval_clone(lval *v);
void lval_del(lval *v);

void lval_print(lval *v);
void lval_print_ln(lval *v);
void lval_print_expr(lval *v, char start, char end);

lval *eval(lval *v);
lval *eval_sexpr(lval *v);
lval *eval_qexpr(lval *v);
lval *lval_read(mpc_ast_t *tree);
lval *builtin_op(lval *v, Operator *s);

Operator *ops_mapper(const char *sym);

lval *must_be_number(lval **operands, int n);
lval *must_be_qexpr(lval **operands, int n);
