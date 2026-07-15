#include "my_lisp.h"
#include <stdlib.h>
#include <string.h>

list *new_func_list(void) { return new_list(); }

void func_list_del(list *l) {
    list_del(l, func_del);
}

void func_del(void *v) {
    func_map *f = v;
    free(f->sym);
    free(f);
}

func_map *new_func(char *sym, builtin_f func) {
    func_map *b = malloc(sizeof(*b));
    b->sym = strdup(sym);
    b->func = func;
    return b;
}

void func_add(list *l, char *sym, builtin_f func) {
    func_map *b = new_func(sym, func);
    list_push(l, b);
}
