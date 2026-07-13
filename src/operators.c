#include "grammer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

const char *ERR_DIV_BY_ZERO = "can't divide by zero!";

lval *must_be_number(lval **operands, int n) {
    for (int i = 1; i < n; i++)
        if (operands[i]->type != LVAL_NUM)
            return new_lval_err("function must be passed a number!");
    return NULL;
}
lval *must_be_qexpr(lval **operands, int n) {
    lval *x = operands[0];
    for (int i = 0; i < n; i++)
        if (x->type != LVAL_QEXPR)
            return new_lval_err("function must be passed a q-expression!");
    return NULL;
}

lval *op_add(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;
    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        x += y;
    }
    return new_lval_num(x);
}
lval *op_sub(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;

    if (n == 1)
        x = -x;

    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        x -= y;
    }
    return new_lval_num(x);
}
lval *op_mul(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;
    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        x *= y;
    }
    return new_lval_num(x);
}
lval *op_div(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;
    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        if (y == 0)
            return new_lval_err(ERR_DIV_BY_ZERO);
        x /= y;
    }
    return new_lval_num(x);
}

lval *op_mod(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;
    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        if (y == 0)
            return new_lval_err(ERR_DIV_BY_ZERO);
        x = fmod(x, y);
    }
    return new_lval_num(x);
}
lval *op_pow(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;
    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        x = pow(x, y);
    }
    return new_lval_num(x);
}
lval *op_max(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;
    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        x = x > y ? x : y;
    }
    return new_lval_num(x);
}
lval *op_min(lval **operands, int n) {
    lval *e = must_be_number(operands, n);
    if (e)
        return e;

    double x = operands[0]->num;
    for (int i = 1; i < n; i++) {
        double y = operands[i]->num;
        x = x < y ? x : y;
    }
    return new_lval_num(x);
}

lval *op_head(lval **operands, int n) {
    if (n != 1)
        return new_lval_err("function head passed too many args!");

    lval *e = must_be_qexpr(operands, n);
    if (e)
        return e;

    lval *x = operands[0];
    if (x->count == 1)
        return new_lval_err("function head passed {}!");

    lval *q = new_lval_qexpr();
    lval_add(q, lval_clone(x->cell[0]));
    return q;
}
lval *op_tail(lval **operands, int n) {
    if (n != 1)
        return new_lval_err("function tail passed too many args!");

    lval *e = must_be_qexpr(operands, n);
    if (e)
        return e;

    lval *x = operands[0];
    if (x->count == 1)
        return new_lval_err("function head passed {}!");

    lval *q = new_lval_qexpr();
    for (int i = 1; i < x->count; i++)
        lval_add(q, lval_clone(x->cell[i]));

    return q;
}
lval *op_list(lval **operands, int n) {
    lval *x = new_lval_qexpr();
    for (int i = 0; i < n; i++)
        lval_add(x, lval_clone(operands[i]));
    return x;
}
lval *op_eval(lval **operands, int n) {
    if (n != 1)
        return new_lval_err("function tail passed too many args!");

    lval *e = must_be_qexpr(operands, n);
    if (e)
        return e;

    lval *x = new_lval_qexpr();

    lval_add(x, eval_sexpr(lval_clone(operands[0])));
    return x;
}
lval *op_join(lval **operands, int n) {
    lval *e = must_be_qexpr(operands, n);
    if (e)
        return e;

    lval *x = new_lval_qexpr();
    for (int i = 0; i < n; i++) {
        lval *curr = operands[i];
        for (int i = 0; i < curr->count; i++) {
            lval_add(x, lval_clone(curr->cell[i]));
        }
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
