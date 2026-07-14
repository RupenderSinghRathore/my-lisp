#include "grammer.h"
#include <stdlib.h>

// create new list
list *new_list(void) {
    list *l = malloc(sizeof(*l));
    l->len = 0;
    l->arr = NULL;
    return l;
}

// free up list
void list_del(list *l) {
    for (int i = 0; i < l->len; i++) {
        lval_del(l->arr[i]);
    }
    free(l->arr);
    free(l);
}

// append v to list
void list_push(list *l, lval *v) {
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

// clones the list
list *list_clone(list *l) {
    list *new = new_list();
    for (int i = 0; i < l->len; i++) {
        list_push(new, lval_clone(l->arr[i]));
    }
    return new;
}
