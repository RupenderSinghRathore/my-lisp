#include "my_lisp.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

const char *ERR_DIV_BY_ZERO = "can't divide by zero!";

typedef double (*reduce_fn)(double a, double b, int i);

lval *must_be_number(list *operands) {
    for (int i = 0; i < operands->len; i++)
        if (((lval *)operands->arr[i])->type != LVAL_NUM)
            return new_lval_err("function must be passed a number!");
    return NULL;
}
lval *must_be_qexpr(list *operands) {
    for (int i = 0; i < operands->len; i++)
        if (((lval *)operands->arr[i])->type != LVAL_QEXPR)
            return new_lval_err("function must be passed a q-expression!");
    return NULL;
}

lval *op_arith(list *operands, char *sym) {

    assert(operands->len != 0);

    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = ((lval *)operands->arr[0])->num;

    if (operands->len == 1 && strcmp(sym, "-") == 0)
        return new_lval_num(-x);

    for (int i = 1; i < operands->len; i++) {
        double y = ((lval *)operands->arr[i])->num;
        if (strcmp(sym, "+") == 0)
            x += y;
        else if (strcmp(sym, "-") == 0)
            x -= y;
        else if (strcmp(sym, "*") == 0)
            x *= y;
        else if (strcmp(sym, "/") == 0) {
            if (y == 0)
                return new_lval_err(ERR_DIV_BY_ZERO);
            x /= y;
        } else if (strcmp(sym, "%") == 0) {
            if (y == 0)
                return new_lval_err(ERR_DIV_BY_ZERO);
            x = fmod(x, y);
        } else if (strcmp(sym, "^") == 0)
            x = pow(x, y);
        else if (strcmp(sym, "max") == 0)
            x = (x > y) ? x : y;
        else if (strcmp(sym, "min") == 0)
            x = (x < y) ? x : y;
    }
    return new_lval_num(x);
}

#define DEFINE_ARITH_BUILTIN(name, op)                                         \
    lval *name(list *f, list *args) {                                          \
        (void)f;                                                               \
        return op_arith(args, op);                                             \
    }

DEFINE_ARITH_BUILTIN(op_add, "+")
DEFINE_ARITH_BUILTIN(op_sub, "-")
DEFINE_ARITH_BUILTIN(op_mul, "*")
DEFINE_ARITH_BUILTIN(op_div, "/")
DEFINE_ARITH_BUILTIN(op_mod, "%")
DEFINE_ARITH_BUILTIN(op_pow, "^")
DEFINE_ARITH_BUILTIN(op_max, "max")
DEFINE_ARITH_BUILTIN(op_min, "min")

lval *op_head(list *f, list *operands) {
    (void)f;
    if (operands->len != 1)
        return new_lval_err("function head passed too many args!");

    lval *e = must_be_qexpr(operands);
    if (e)
        return e;

    lval *x = operands->arr[0];
    if (x->cell->len == 0)
        return new_lval_err("function head passed {}!");

    lval *q = new_lval_qexpr();
    list_push(q->cell, list_pop_left(x->cell));
    return q;
}
lval *op_tail(list *f, list *operands) {
    (void)f;
    if (operands->len != 1)
        return new_lval_err("function tail passed too many args!");

    lval *e = must_be_qexpr(operands);
    if (e)
        return e;

    lval *x = operands->arr[0];
    if (x->cell->len == 0)
        return new_lval_err("function tail passed {}!");

    lval *q = new_lval_qexpr();
    lval_del(list_pop_left(x->cell));
    list_del(q->cell, lval_del);
    q->cell = x->cell;
    x->cell = NULL;

    return q;
}
lval *op_list(list *f, list *operands) {
    (void)f;
    lval *x = new_lval_qexpr();
    while (operands->len > 0)
        list_push(x->cell, list_pop_left(operands));
    return x;
}
lval *op_eval(list *f, list *operands) {
    if (operands->len != 1)
        return new_lval_err("function eval passed too many args!");

    lval *e = must_be_qexpr(operands);
    if (e)
        return e;

    lval *exp = list_pop_left(operands);
    exp->type = LVAL_SEXPR;
    return eval(f, exp);
}
lval *op_join(list *f, list *operands) {
    (void)f;
    lval *e = must_be_qexpr(operands);
    if (e)
        return e;

    lval *x = new_lval_qexpr();
    for (int i = 0; i < operands->len; i++) {
        lval *curr = operands->arr[i];
        while (curr->cell->len > 0)
            list_push(x->cell, list_pop_left(curr->cell));
    }
    return x;
}

func_map ops[] = {
    {"+", op_add},     {"-", op_sub},     {"*", op_mul},     {"/", op_div},
    {"%", op_mod},     {"^", op_pow},     {"max", op_max},   {"min", op_min},
    {"head", op_head}, {"tail", op_tail}, {"list", op_list}, {"eval", op_eval},
    {"join", op_join},
};

void add_builtin_funcs(list *l) {
    func_add(l, "+", op_add);
    func_add(l, "-", op_sub);
    func_add(l, "*", op_mul);
    func_add(l, "/", op_div);
    func_add(l, "%", op_mod);
    func_add(l, "^", op_pow);
    func_add(l, "max", op_max);
    func_add(l, "min", op_min);

    func_add(l, "head", op_head);
    func_add(l, "tail", op_tail);
    func_add(l, "list", op_list);
    func_add(l, "eval", op_eval);
    func_add(l, "join", op_join);
}

lval *ops_mapper(list *l, lval *v) {
    for (int i = 0; i < l->len; i++) {
        func_map *curr = l->arr[i];
        if (strcmp(curr->sym, v->sym) == 0) {
            return new_lval_func(curr->func);
        }
    }
    return new_lval_err("unknown symbol!");
}

void operator_del(func_map *op) {
    if (op)
        free(op->sym);
    free(op);
}
