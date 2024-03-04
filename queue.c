#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

#define STREQ(left, right) (!strcmp((left), (right)))

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        free(entry->value), entry->value = NULL;
        free(entry), entry = NULL;
    }

    free(head), head = NULL;
}

/* Create a new queue element with a given string */
static inline bool q_insert_node(struct list_head *head, char *s)
{
    element_t *elem = malloc(sizeof(element_t));
    if (!elem)
        return false;

    elem->value = strdup(s);
    if (!elem->value) {
        free(elem), elem = NULL;
        return false;
    }
    list_add(&elem->list, head);

    return true;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    return q_insert_node(head, s);
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return q_insert_node(head->prev, s);
}

/* Remove an element from a queue */
element_t *q_remove_node(struct list_head *node, char *sp, size_t bufsize)
{
    if (!node)
        return NULL;

    element_t *entry = list_entry(node, element_t, list);
    list_del_init(node);

    if (sp && bufsize) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return entry;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    return q_remove_node(head->next, sp, bufsize);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    return q_remove_node(head->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int cnt = 0;
    struct list_head *node;
    list_for_each (node, head)
        cnt++;
    return cnt;
}

static struct list_head *list_find_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return NULL;

    struct list_head *fast = head->next, *slow;
    list_for_each (slow, head) {
        if (fast->next == head || fast->next->next == head)
            break;
        fast = fast->next->next;
    }
    return slow;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *mid = list_find_mid(head);
    list_del_init(mid);

    element_t *entry = list_entry(mid, element_t, list);
    free(entry->value), entry->value = NULL;
    free(entry), entry = NULL;
    return true;
}


/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    if (list_empty(head) || list_is_singular(head))
        return true;

    struct list_head *prev = head->next, *curr = prev->next, *next = curr->next;
    element_t *prev_entry = list_entry(prev, element_t, list);

    bool dup = false;
    while (curr != head) {
        element_t *curr_entry = list_entry(curr, element_t, list);
        prev_entry = list_entry(prev, element_t, list);

        if (!strcmp(curr_entry->value, prev_entry->value)) {
            list_del(curr);
            q_release_element(curr_entry);
            dup = true;
        } else {
            if (dup) {
                list_del(prev);
                q_release_element(prev_entry);
                dup = false;
            }
            prev = curr;
        }
        curr = next;
        next = next->next;
    }
    if (dup) {
        list_del(prev);
        q_release_element(prev_entry);
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head)
            break;
        list_move_tail(node->next, node);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        list_move(node, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head) || k <= 1)
        return;

    int i = 0;
    struct list_head *node, *safe, *head_from = head, head_to;
    list_for_each_safe (node, safe, head) {
        if (++i < k)
            continue;

        list_cut_position(&head_to, head_from, node);
        q_reverse(&head_to);
        list_splice(&head_to, head_from);
        head_from = safe->prev;
        i = 0;
    }
}

/**
 * elem_compare - Compare two elements in ascending/descending order
 * @a: first element to be compared
 * @b: second element to be compared
 * @descend: whether or not to sort in descending order
 *
 * Return: negative number if @a is former than @b
 *         positive number if @a is latter than @b
 *         zero if @a is equal to @b
 */
static int elem_compare(const element_t *a, const element_t *b, bool descend)
{
    int res = strcmp(a->value, b->value);
    return descend ? -res : res;
}

/**
 * list_compare - Compare two list nodes in ascending/descending order
 * @a: first node to be compared
 * @b: second node to be compared
 * @descend: whether or not to sort in descending order
 *
 * Return: negative number if @a is former than @b
 *         positive number if @a is latter than @b
 *         zero if @a is equal to @b
 */
static int list_compare(const struct list_head *a,
                        const struct list_head *b,
                        bool descend)
{
    element_t *e1 = list_first_entry(a, element_t, list);
    element_t *e2 = list_first_entry(b, element_t, list);
    return elem_compare(e1, e2, descend);
}

// /* Merge list l2 into list l1 in ascending/descending order */
static void list_merge(struct list_head *l1, struct list_head *l2, bool descend)
{
    LIST_HEAD(sorted);
    struct list_head *next, **curr = NULL;
    for (; !list_empty(l1) && !list_empty(l2); *curr = next) {
        curr = list_compare(l1, l2, descend) <= 0 ? &l1->next : &l2->next;
        next = (*curr)->next;
        list_move_tail(*curr, &sorted);
    }

    list_splice_tail_init(list_empty(l1) ? l2 : l1, &sorted);
    list_splice_tail(&sorted, l1);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *mid = list_find_mid(head);
    LIST_HEAD(left);
    list_cut_position(&left, head, mid);
    q_sort(&left, descend);
    q_sort(head, descend);
    list_merge(head, &left, descend);
}

#define list_for_each_safe_reverse(node, safe, head)             \
    for (node = (head)->prev, safe = node->prev; node != (head); \
         node = safe, safe = node->prev)

#define list_for_each_entry_safe_reverse(entry, safe, head, member)        \
    for (entry = list_entry((head)->prev, __typeof__(*entry), member),     \
        safe = list_entry(entry->member.prev, __typeof__(*entry), member); \
         &entry->member != (head); entry = safe,                           \
        safe = list_entry(safe->member.prev, __typeof__(*entry), member))

int q_monotonic(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    int size = 0;
    element_t *entry, *safe,
        *next_entry = list_entry(head->prev, element_t, list);
    list_for_each_entry_safe_reverse(entry, safe, head, list)
    {
        if (elem_compare(entry, next_entry, descend) <= 0) {
            next_entry = entry;
            size++;
        } else {
            // Remove current entry
            list_del(&entry->list);
            free(entry->value), entry->value = NULL;
            free(entry), entry = NULL;
        }
    }
    return size;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return q_monotonic(head, false);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    return q_monotonic(head, true);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head)
        return 0;
    LIST_HEAD(sorted);
    queue_contex_t *curr = NULL;
    queue_contex_t *last = list_last_entry(head, queue_contex_t, chain);

    list_for_each_entry (curr, head, chain) {
        if (curr == last) {
            list_splice_init(curr->q, &sorted);
            break;
        }
        list_splice_init(curr->q, &sorted);
        list_splice_init(last->q, &sorted);
        last = list_entry(last->chain.prev, queue_contex_t, chain);
    }
    q_sort(&sorted, descend);
    int size = q_size(&sorted);
    list_splice_init(&sorted, list_first_entry(head, queue_contex_t, chain)->q);
    return size;
}
