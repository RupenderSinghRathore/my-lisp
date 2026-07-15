#include "my_lisp.h"
#include <stdlib.h>

// create new list
list *new_list(void) {
    list *l = malloc(sizeof(*l));
    l->len = 0;
    l->arr = NULL;
    return l;
}

// free up list
void list_del(list *l, list_ele_del del) {
    if (!l)
        return;

    if (del)
        for (int i = 0; i < l->len; i++)
            del(l->arr[i]);

    free(l->arr);
    free(l);
}

// append v to list
void list_push(list *l, void *v) {
    l->len++;
    l->arr = realloc(l->arr, sizeof(void *) * l->len);
    l->arr[l->len - 1] = v;
}

// takes ownership of ith lval and returns it
lval *list_take(list *l, int i) {
    lval *x = l->arr[i];

    int elements_to_move = l->len - i - 1;
    if (elements_to_move != 0)
        memmove(&l->arr[i], &l->arr[i + 1], sizeof(void *) * elements_to_move);

    l->len--;

    l->arr = realloc(l->arr, sizeof(void *) * l->len);
    return x;
}

lval *list_pop_left(list *l) {
    if (l->len == 0)
        NULL;
    return list_take(l, 0);
}
lval *list_pop(list *l) {
    if (l->len == 0)
        NULL;
    return list_take(l, l->len - 1);
}

// clones the list
list *list_clone(list *l, list_ele_clone clone) {
    list *n = new_list();
    for (int i = 0; i < l->len; i++) {
        if (clone)
            list_push(n, clone(l->arr[i]));
        else
            list_push(n, l->arr[i]);
    }
    return n;
}
