#include "grammer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

const char *ERR_DIV_BY_ZERO = "can't divide by zero!";

lval *must_be_number(list *operands) {
    for (int i = 1; i < operands->len; i++)
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

lval *op_add(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        x += y;
    }
    return new_lval_num(x);
}
lval *op_sub(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;

    if (operands->len == 1)
        x = -x;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        x -= y;
    }
    return new_lval_num(x);
}
lval *op_mul(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        x *= y;
    }
    return new_lval_num(x);
}
lval *op_div(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        if (y == 0)
            return new_lval_err(ERR_DIV_BY_ZERO);
        x /= y;
    }
    return new_lval_num(x);
}

lval *op_mod(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        if (y == 0)
            return new_lval_err(ERR_DIV_BY_ZERO);
        x = fmod(x, y);
    }
    return new_lval_num(x);
}
lval *op_pow(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        x = pow(x, y);
    }
    return new_lval_num(x);
}
lval *op_max(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        x = x > y ? x : y;
    }
    return new_lval_num(x);
}
lval *op_min(list *operands) {
    lval *e = must_be_number(operands);
    if (e)
        return e;

    double x = operands->arr[0]->num;
    for (int i = 1; i < operands->len; i++) {
        double y = operands->arr[i]->num;
        x = x < y ? x : y;
    }
    return new_lval_num(x);
}

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
    list_push(q->cell, list_take(x->cell, 0));
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
    while (x->cell->len > 1)
        list_push(q->cell, list_take(x->cell, 1));

    return q;
}
lval *op_list(list *operands) {
    lval *x = new_lval_qexpr();
    while (operands->len > 0)
        list_push(x->cell, list_take(operands, 0));
    return x;
}
lval *op_eval(list *operands) {
    if (operands->len != 1)
        return new_lval_err("function tail passed too many args!");

    lval *e = must_be_qexpr(operands);
    if (e)
        return e;

    lval *x = new_lval_qexpr();

    list_push(x->cell, eval_sexpr(list_take(operands, 0)));
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
            list_push(x->cell, list_take(curr->cell, 0));
    }
    return x;
}

Operator ops[] = {
    {"+", op_add},
    {
        "-",
        op_sub,
    },
    {
        "*",
        op_mul,
    },
    {
        "/",
        op_div,
    },
    {
        "%",
        op_mod,
    },
    {
        "^",
        op_pow,
    },
    {
        "max",
        op_max,
    },
    {
        "min",
        op_min,
    },
    {
        "head",
        op_head,
    },
    {
        "tail",
        op_tail,
    },
    {
        "list",
        op_list,
    },
    {
        "eval",
        op_eval,
    },
    {
        "join",
        op_join,
    },
};

Operator *ops_mapper(const char *sym) {
    int n = sizeof(ops) / sizeof(Operator);
    Operator *o = malloc(sizeof(*o));
    for (int i = 0; i < n; i++) {
        if (strcmp(ops[i].sym, sym) == 0) {
            o->sym = strdup(sym);
            o->eval = ops[i].eval;
            return o;
        }
    }
    return o;
}
