#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// =============================================================================
// DOUBLY LINKED LIST
//
// Each node has both a next and prev pointer. The list tracks both head and
// tail, giving O(1) insert and remove at either end without traversal.
//
// Extra cost over singly: twice the pointer storage per node, slightly more
// bookkeeping on insert/remove. Worth it when you need backward traversal or
// O(1) tail operations (e.g. LRU cache, browser history).
// =============================================================================

typedef struct Node {
    int         val;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct {
    Node  *head;
    Node  *tail;
    size_t length;
} List;

List list_init(void) { return (List){0}; }

static Node *node_create(int val) {
    Node *n = malloc(sizeof(Node));
    n->val  = val;
    n->prev = NULL;
    n->next = NULL;
    return n;
}

// Insert at front — O(1)
void list_push_front(List *l, int val) {
    Node *n = node_create(val);
    n->next = l->head;
    if (l->head) l->head->prev = n;
    else         l->tail = n;
    l->head = n;
    l->length++;
}

// Insert at back — O(1)
void list_push_back(List *l, int val) {
    Node *n = node_create(val);
    n->prev = l->tail;
    if (l->tail) l->tail->next = n;
    else         l->head = n;
    l->tail = n;
    l->length++;
}

// Remove from front — O(1)
bool list_pop_front(List *l, int *out) {
    if (!l->head) return false;
    Node *old = l->head;
    *out      = old->val;
    l->head   = old->next;
    if (l->head) l->head->prev = NULL;
    else         l->tail = NULL;
    free(old);
    l->length--;
    return true;
}

// Remove from back — O(1)
bool list_pop_back(List *l, int *out) {
    if (!l->tail) return false;
    Node *old = l->tail;
    *out      = old->val;
    l->tail   = old->prev;
    if (l->tail) l->tail->next = NULL;
    else         l->head = NULL;
    free(old);
    l->length--;
    return true;
}

// Remove a specific node directly (no search needed if you have the pointer) — O(1)
void list_unlink(List *l, Node *n) {
    if (n->prev) n->prev->next = n->next; else l->head = n->next;
    if (n->next) n->next->prev = n->prev; else l->tail = n->prev;
    free(n);
    l->length--;
}

// Delete first node with matching value — O(n)
bool list_delete(List *l, int val) {
    Node *cur = l->head;
    while (cur) {
        if (cur->val == val) { list_unlink(l, cur); return true; }
        cur = cur->next;
    }
    return false;
}

Node *list_find(List *l, int val) {
    Node *cur = l->head;
    while (cur) {
        if (cur->val == val) return cur;
        cur = cur->next;
    }
    return NULL;
}

void list_free(List *l) {
    Node *cur = l->head;
    while (cur) { Node *next = cur->next; free(cur); cur = next; }
    l->head = l->tail = NULL;
    l->length = 0;
}

void list_print_forward(List *l) {
    Node *cur = l->head;
    while (cur) { printf("%d", cur->val); if (cur->next) printf(" <-> "); cur = cur->next; }
    printf("  (length=%zu)\n", l->length);
}

void list_print_backward(List *l) {
    Node *cur = l->tail;
    while (cur) { printf("%d", cur->val); if (cur->prev) printf(" <-> "); cur = cur->prev; }
    printf("  (backward)\n");
}


int main(void) {
    List l = list_init();

    printf("=== Push back ===\n");
    list_push_back(&l, 10);
    list_push_back(&l, 20);
    list_push_back(&l, 30);
    list_print_forward(&l);

    printf("\n=== Push front ===\n");
    list_push_front(&l, 5);
    list_push_front(&l, 1);
    list_print_forward(&l);   // 1 <-> 5 <-> 10 <-> 20 <-> 30

    printf("\n=== Forward and backward traversal ===\n");
    list_print_forward(&l);
    list_print_backward(&l);

    printf("\n=== Pop front and back ===\n");
    int val;
    list_pop_front(&l, &val); printf("pop front: %d  |  ", val); list_print_forward(&l);
    list_pop_back(&l, &val);  printf("pop back:  %d  |  ", val); list_print_forward(&l);

    printf("\n=== Delete middle ===\n");
    list_delete(&l, 10);
    list_print_forward(&l);

    printf("\n=== Direct unlink via pointer (O(1)) ===\n");
    Node *n = list_find(&l, 20);
    if (n) list_unlink(&l, n);   // no search on removal — O(1)
    list_print_forward(&l);

    list_free(&l);
    return 0;
}
