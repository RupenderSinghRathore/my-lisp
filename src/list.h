typedef struct {
    int len;
    void **arr;
} list;

typedef void (*list_ele_del)(void *);
typedef void *(*list_ele_clone)(void *);

list *new_list(void);
void list_del(list *l, list_ele_del del);
void list_push(list *l, void *v);
void *list_take(list *l, int i);
void *list_pop_left(list *l);
void *list_pop(list *l);
list *list_clone(list *l, list_ele_clone clone);
