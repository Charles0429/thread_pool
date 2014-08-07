#ifndef _LIST_H
#define _LIST_H


struct list_head
{
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
    new->next = next;
    new->prev = prev;
    prev->next = new;
    next->prev = new;
}

static inline void list_add(struct list_head *new,
                            struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new,
                                 struct list_head *head)
{
    __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev,
                              struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void __list_del_entry(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void list_del_init(struct list_head *entry)
{
    __list_del_entry(entry);
    INIT_LIST_HEAD(entry);
}

static inline void list_replace(struct list_head *old,
                                struct list_head *new)
{
    new->next = old->next;
    new->prev = old->prev;
    new->prev->next = new;
    new->next->prev = new;
}

static inline void list_replace_init(struct list_head *old,
                                     struct list_head *new)
{
    list_replace(old, new);
    INIT_LIST_HEAD(old);
}

static inline void list_move(struct list_head *list,
                             struct list_head *head)
{
    __list_del_entry(list);
    list_add(list, head);
}

static inline void list_move_tail(struct list_head *list,
                                  struct list_head *head)
{
    __list_del_entry(list);
    list_add_tail(list, head);
}

static inline int list_is_last(const struct list_head *list,
                               const struct list_head *head)
{
    return list->next == head;
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

#define offsetof(type, member) \
    ((size_t)&(((type *)0)->member))

#define container_of(ptr, type, member) \
({                                      \
    const typeof( ((type *)0)->member ) *_mptr = (ptr); \
    (type *)((char *)_mptr - offsetof(type, member)); \
})

#define list_entry(list_ptr, type, member) \
    container_of(list_ptr, type, member)

#define list_first_entry(list_ptr, type, member) \
    list_entry(list_ptr->next, type, member)

#define list_last_entry(list_ptr, type, member) \
    list_entry(list_ptr->prev, type, member)

#define list_first_entry_or_null(list_ptr, type, member) \
    (list_empty(list_ptr)? NULL : list_first_entry(list_ptr, type, member))

#define list_next_entry(elem_ptr, member) \
    list_entry((elem_ptr)->member.next, typeof(*(elem_ptr)), member)

#define list_prev_entry(elem_ptr, member) \
    list_entry((elem_ptr)->member.prev, typeof(*(elem_ptr)), member)

#define list_for_each(list_ptr, head) \
    for (list_ptr = (head)->next; list_ptr != (head); list_ptr = list_ptr->next)

#define list_for_each_prev(list_ptr, head) \
    for (list_ptr = (head)->prev; list_ptr != (head); list_ptr = list_ptr->prev)

#define list_for_each_safe(list_ptr, list_next_ptr, head) \
    for (list_ptr = (head)->next, list_next_ptr = list_ptr->next; list_ptr != (head); list_ptr = list_next_ptr, list_next_ptr = list_next_ptr->next)

#define list_for_each_prev_safe(list_ptr, list_prev_ptr, head) \
    for (list_ptr = (head)->prev, list_prev_ptr = list_ptr->prev; list_ptr != (head); list_ptr = list_prev_ptr, list_prev_ptr = list_prev_ptr->prev)

#define list_for_each_entry(elem_ptr, head, member) \
    for(elem_ptr = list_first_entry(head, typeof(*elem_ptr), member); \
        &(elem_ptr->member) != (head); \
        elem_ptr = list_next_entry(elem_ptr, member))

#define list_for_entry_reverse(elem_ptr, head, member) \
    for (elem_ptr = list_last_entry(head, typeof(*elem_ptr), member); \
         &(elem_ptr->member) != (head); \
         elem_ptr = list_prev_entry(elem_ptr, member))

#define list_prepare_entry(elem_ptr, list_ptr, member) \
        (list_ptr ? list_entry(list_ptr, typeof(*elem_ptr), member) : NULL)

#define list_for_each_entry_continue(elem_ptr, head, member) \
    for(elem_ptr = list_next_entry(elem_ptr, member); \
        &elem_ptr->member != (head) ;  \
        elem_ptr = list_next_entry(elem_ptr, member))

#define list_for_each_continue_reverse(elem_ptr, head, member) \
    for(elem_ptr = list_prev_entry(elem_ptr, member);  \
        &elem_ptr->member != (head); \
        elem_ptr = list_prev_entry(elem_ptr, member))

#define list_for_each_entry_from(elem_ptr, head, member) \
    for( ; &elem_ptr->member != (head); \
        elem_ptr = list_next_entry(elem_ptr, member))

#define list_for_each_entry_safe(elem_ptr, elem_next_ptr, head, member) \
    for(elem_ptr = list_first_entry(head, typeof(*elem_ptr), member), \
        elem_next_ptr = list_next_entry(elem_ptr, member); \
        &elem_ptr->member != (head); \
        elem_ptr = elem_next_ptr, elem_next_ptr = list_next_entry(elem_next_ptr, member))

#define list_for_each_entry_safe_continue(elem_ptr, elem_next_ptr, head, member) \
    for(elem_ptr = list_next_entry(elem_ptr, member), \
        elem_next_ptr = list_next_entry(elem_ptr, member); \
        &elem_ptr->member != (head); \
        elem_ptr = elem_next_ptr, elem_next_ptr = list_next_entry(elem_next_ptr, member))

#define list_for_each_entry_safe_from(elem_ptr, elem_next_ptr, head, member) \
    for(elem_next_ptr = list_next_entry(elem_ptr, member); \
        &elem_ptr->member != (head); \
        elem_ptr = elem_next_ptr, elem_next_ptr = list_next_entry(elem_next_ptr, member))

#define list_for_each_entry_safe_reverse(elem_ptr, elem_next_ptr, head, member) \
    for(elem_ptr = list_last_entry(head, typeof(*elem_ptr), member), \
        elem_next_ptr = list_prev_entry(elem_ptr, member); \
        &elem_ptr->member != (head); \
        elem_ptr = elem_next_ptr, elem_next_ptr = list_prev_entry(elem_next_ptr, member))

#endif
