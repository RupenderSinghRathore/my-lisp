#include "my_lisp.h"
#include <assert.h>
#include <math.h>
#include <string.h>

const char *ERR_DIV_BY_ZERO = "can't divide by zero!";

typedef double (*reduce_fn)(double a, double b, int i);

lval *type_err(const char *fn, lval_t got, lval_t want) {
    return new_lval_err("Function '%s' passed incorrect argument. got "
                        "%s, expected: %s.",
                        fn, lval_type(got), lval_type(want));
}

lval *must_be_type(list *operands, const char *fn, lval_t want) {
    for (int i = 0; i < operands->len; i++) {
        lval *curr = operands->arr[i];
        if (curr->type != want)
            return type_err(fn, curr->type, want);
    }
    return NULL;
}

lval *must_be_number(list *operands, const char *fn) {
    return must_be_type(operands, fn, LVAL_NUM);
}
lval *must_be_qexpr(list *operands, const char *fn) {
    return must_be_type(operands, fn, LVAL_QEXPR);
}

lval *must_be_symbol(list *operands) {
    for (int i = 0; i < operands->len; i++)
        if (((lval *)operands->arr[i])->type != LVAL_SYM)
            return new_lval_err("function must be passed a symbol!");
    return NULL;
}

lval *op_arith(list *operands, char *sym) {

    assert(operands->len != 0);

    lval *e = must_be_number(operands, sym);
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
    lval *name(list *env, list *args) {                                        \
        (void)env;                                                             \
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

#define ASSERT_NO_OF_ARGS(fn, got, expected)                                   \
    do {                                                                       \
        if ((got) != (expected))                                               \
            return new_lval_err(                                               \
                "function '%s' passed incorrect number of arguments. "         \
                "got %i, expected %d.",                                        \
                (fn), (got), (expected));                                      \
    } while (0);

lval *op_head(list *env, list *operands) {
    (void)env;

    ASSERT_NO_OF_ARGS("head", operands->len, 1);

    lval *e = must_be_qexpr(operands, "head");
    if (e)
        return e;

    lval *x = operands->arr[0];
    if (x->cell->len == 0)
        return new_lval_err("function head passed {}!");

    lval *q = new_lval_qexpr();
    list_push(q->cell, list_pop_left(x->cell));
    return q;
}
lval *op_tail(list *env, list *operands) {
    (void)env;

    ASSERT_NO_OF_ARGS("tail", operands->len, 1);

    lval *e = must_be_qexpr(operands, "tail");
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
lval *op_list(list *env, list *operands) {
    (void)env;
    lval *x = new_lval_qexpr();
    while (operands->len > 0)
        list_push(x->cell, list_pop_left(operands));
    return x;
}
lval *op_eval(list *f, list *operands) {
    ASSERT_NO_OF_ARGS("eval", operands->len, 1);

    lval *e = must_be_qexpr(operands, "eval");
    if (e)
        return e;

    lval *exp = list_pop_left(operands);
    exp->type = LVAL_SEXPR;
    return eval(f, exp);
}
lval *op_join(list *env, list *operands) {
    (void)env;
    lval *e = must_be_qexpr(operands, "join");
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
lval *op_def(list *env, list *operands) {
    assert(operands->len != 0);

    lval *result = NULL;

    lval *exp = list_pop_left(operands);
    if (exp->type != LVAL_QEXPR) {
        result = type_err("def", exp->type, LVAL_QEXPR);
        goto cleanup;
    }

    if (exp->cell->len != operands->len) {
        result = new_lval_err(
            "function '%s' passed unequal number of arguments to variable",
            "def");
        goto cleanup;
    }

    lval *e = must_be_number(operands, "def");
    if (e) {
        result = e;
        goto cleanup;
    }
    e = must_be_symbol(exp->cell);
    if (e) {
        result = e;
        goto cleanup;
    }

    for (int i = 0; i < exp->cell->len; i++) {
        lval *curr = exp->cell->arr[i];
        env_update_or_add(env, curr->sym, list_pop_left(operands));
    }
    result = new_lval_sexpr();

cleanup:
    lval_del(exp);
    return result;
}

void register_func(list *env, char *sym, builtin_f func) {
    env_update_or_add(env, sym, new_lval_func(func));
}

void add_builtin_funcs(list *l) {
    register_func(l, "+", op_add);
    register_func(l, "-", op_sub);
    register_func(l, "*", op_mul);
    register_func(l, "/", op_div);
    register_func(l, "%", op_mod);
    register_func(l, "^", op_pow);
    register_func(l, "max", op_max);
    register_func(l, "min", op_min);

    register_func(l, "head", op_head);
    register_func(l, "tail", op_tail);
    register_func(l, "list", op_list);
    register_func(l, "eval", op_eval);
    register_func(l, "join", op_join);
    register_func(l, "def", op_def);
}
