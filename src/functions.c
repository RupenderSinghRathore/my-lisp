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

lval *op_add(list *operands) { return op_arith(operands, "+"); }
lval *op_sub(list *operands) { return op_arith(operands, "-"); }
lval *op_mul(list *operands) { return op_arith(operands, "*"); }
lval *op_div(list *operands) { return op_arith(operands, "/"); }
lval *op_mod(list *operands) { return op_arith(operands, "%"); }
lval *op_pow(list *operands) { return op_arith(operands, "^"); }
lval *op_max(list *operands) { return op_arith(operands, "max"); }
lval *op_min(list *operands) { return op_arith(operands, "min"); }

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
    list_del(q->cell, lval_del);
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

builtin ops[] = {
    {"+", op_add},     {"-", op_sub},     {"*", op_mul},     {"/", op_div},
    {"%", op_mod},     {"^", op_pow},     {"max", op_max},   {"min", op_min},
    {"head", op_head}, {"tail", op_tail}, {"list", op_list}, {"eval", op_eval},
    {"join", op_join},
};

builtin *ops_mapper(const char *sym) {
    int n = sizeof(ops) / sizeof(builtin);
    for (int i = 0; i < n; i++) {
        if (strcmp(ops[i].str, sym) == 0) {
            builtin *o = malloc(sizeof(*o));
            o->str = strdup(sym);
            o->eval = ops[i].eval;
            return o;
        }
    }
    return NULL;
}

void operator_del(builtin *op) {
    if (op)
        free(op->str);
    free(op);
}
