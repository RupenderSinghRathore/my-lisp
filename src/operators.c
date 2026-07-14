#include "my_lisp.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

const char *ERR_DIV_BY_ZERO = "can't divide by zero!";

typedef double (*reduce_fn)(double a, double b, int i);

lval *must_be_number(list *operands) {
    for (int i = 0; i < operands->len; i++)
        if (operands->arr[i]->type != LVAL_NUM)
            return new_lval_err("function must be passed a number!");
    return NULL;
}
lval *must_be_qexpr(list *operands) {
    for (int i = 0; i < operands->len; i++)
        if (operands->arr[i]->type != LVAL_QEXPR)
            return new_lval_err("function must be passed a q-expression!");
    return NULL;
}

lval *op_arith(list *operands, reduce_fn reducer, double initial) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = initial;
    for (int i = 0; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        x = reducer(x, y, i);
        if (isnan(x))
            return new_lval_err(ERR_DIV_BY_ZERO);
    }
    return new_lval_num(x);
}

double add_reduce(double a, double x, int _) {
    (void)_;
    return a + x;
}
double sub_reduce(double a, double x, int i) {
    if (i == 0)
        return -x;
    return a - x;
}
double mut_reduce(double a, double x, int _) {
    (void)_;
    return a / x;
}
double div_reduce(double a, double x, int i) {
    if (i == 0)
        return x;
    if (x == 0)
        return NAN;
    return a / x;
}
double mod_reduce(double a, double x, int i) {
    if (i == 0)
        return x;
    if (x == 0)
        return NAN;
    return fmod(a, x);
}
double pow_reduce(double a, double x, int i) {
    if (i == 0)
        return x;
    return pow(a, x);
}
double max_reduce(double a, double x, int i) {
    if (i == 0)
        return x;
    return (a > x) ? a : x;
}
double min_reduce(double a, double x, int i) {
    if (i == 0)
        return x;
    return (a < x) ? a : x;
}

lval *op_add(list *operands) { return op_arith(operands, add_reduce, 0); }
lval *op_sub(list *operands) {
    if (operands->len == 1)
        return op_arith(operands, sub_reduce, -operands->arr[0]->num);
    else
        return op_arith(operands, sub_reduce, 0);
}
lval *op_mul(list *operands) { return op_arith(operands, mut_reduce, 1); }
lval *op_div(list *operands) { return op_arith(operands, div_reduce, 1); }
lval *op_mod(list *operands) { return op_arith(operands, mod_reduce, 1); }
lval *op_pow(list *operands) { return op_arith(operands, pow_reduce, 1); }

lval *op_max(list *operands) { return op_arith(operands, max_reduce, 0); }
lval *op_min(list *operands) { return op_arith(operands, min_reduce, 0); }

lval *op_head(list *operands) {
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
lval *op_tail(list *operands) {
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
    list_del(q->cell);
    q->cell = x->cell;
    x->cell = NULL;

    return q;
}
lval *op_list(list *operands) {
    lval *x = new_lval_qexpr();
    while (operands->len > 0)
        list_push(x->cell, list_pop_left(operands));
    return x;
}
lval *op_eval(list *operands) {
    if (operands->len != 1)
        return new_lval_err("function eval passed too many args!");

    lval *e = must_be_qexpr(operands);
    if (e)
        return e;

    lval *x = new_lval_qexpr();
    lval *exp = list_pop_left(operands);
    exp->type = LVAL_SEXPR;
    list_push(x->cell, eval(exp));
    return x;
}
lval *op_join(list *operands) {
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

Operator ops[] = {
    {"+", op_add},     {"-", op_sub},     {"*", op_mul},     {"/", op_div},
    {"%", op_mod},     {"^", op_pow},     {"max", op_max},   {"min", op_min},
    {"head", op_head}, {"tail", op_tail}, {"list", op_list}, {"eval", op_eval},
    {"join", op_join},
};

Operator *ops_mapper(const char *sym) {
    int n = sizeof(ops) / sizeof(Operator);
    for (int i = 0; i < n; i++) {
        if (strcmp(ops[i].sym, sym) == 0) {
            Operator *o = malloc(sizeof(*o));
            o->sym = strdup(sym);
            o->eval = ops[i].eval;
            return o;
        }
    }
    return NULL;
}
