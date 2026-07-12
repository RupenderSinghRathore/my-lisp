#include "grammer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

const char *ERR_DIV_BY_ZERO = "can't divide by zero!";

lval *op_add(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        x += y;
    }
    return new_lval_num(x);
}
lval *op_sub(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        x -= y;
    }
    return new_lval_num(x);
}
lval *op_mul(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        x *= y;
    }
    return new_lval_num(x);
}
lval *op_div(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        if (y == 0)
            return new_lval_err(ERR_DIV_BY_ZERO);
        x /= y;
    }
    return new_lval_num(x);
}

lval *op_mod(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        if (y == 0)
            return new_lval_err(ERR_DIV_BY_ZERO);
        x = fmod(x, y);
    }
    return new_lval_num(x);
}
lval *op_pow(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        x = pow(x, y);
    }
    return new_lval_num(x);
}
lval *op_max(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        x = x > y ? x : y;
    }
    return new_lval_num(x);
}
lval *op_min(lval *v) {
    double x = v->cell[1]->num;
    for (int i = 2; i < v->count; i++) {
        double y = v->cell[i]->num;
        x = x < y ? x : y;
    }
    return new_lval_num(x);
}

Operator ops[] = {
    {"+", op_add},
    {
        "-", op_sub,
    },
    {
        "*", op_mul,
    },
    {
        "/", op_div,
    },
    {
        "%", op_mod,
    },
    {
        "^", op_pow,
    },
    {
        "max", op_max,
    },
    {
        "min", op_min,
    },
};

Operator *ops_mapper(const char *sym)  {
    int n = sizeof(ops)/ sizeof(Operator);
    Operator* o = malloc(sizeof(*o));
    for (int i = 0; i < n; i++) {
        if (strcmp(ops[i].sym, sym) == 0) {
            o->sym = strdup(sym);
            o->eval = ops[i].eval;
            return o;
        }
    }
    return o;
}
