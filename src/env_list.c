#include "my_lisp.h"
#include <stdlib.h>
#include <string.h>

list *new_env_list(void) { return new_list(); }

void env_list_del(list *l) { list_del(l, env_del); }

void env_del(void *v) {
    func_map *f = v;
    free(f->sym);
    lval_del(f->val);
    free(f);
}

func_map *new_func(char *sym, lval *f) {
    func_map *b = malloc(sizeof(*b));
    b->sym = strdup(sym);
    b->val = f;
    return b;
}

bool env_update(list *l, char *sym, lval *f) {
    for (int i = 0; i < l->len; i++) {
        func_map *curr = l->arr[i];
        if (strcmp(curr->sym, sym) == 0) {
            lval_del(curr->val);
            curr->val = f;
            return true;
        }
    }
    return false;
}

void env_update_or_add(list *l, char *sym, lval *f) {
    if (!env_update(l, sym, f))
        list_push(l, new_func(sym, f));
}

lval *env_mapper(list *l, lval *v) {
    for (int i = 0; i < l->len; i++) {
        func_map *curr = l->arr[i];
        if (strcmp(curr->sym, v->sym) == 0) {
            return lval_clone(curr->val);
        }
    }
    return new_lval_err("unknown symbol '%s'", v->sym);
}
