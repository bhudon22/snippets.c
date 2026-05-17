#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// =============================================================================
// SINGLY LINKED LIST
//
// Each node holds a value and a pointer to the next node. The list owns only
// a pointer to the head. Traversal is one direction only (forward).
//
// Strengths: O(1) insert/remove at head, no pre-allocated capacity needed.
// Weaknesses: O(n) search, O(n) insert/remove at tail, no backward traversal,
//             poor cache locality (nodes scattered in heap memory).
// =============================================================================

typedef struct Node {
    int         val;
    struct Node *next;
} Node;

typedef struct {
    Node  *head;
    size_t length;
} List;

List list_init(void) { return (List){0}; }

static Node *node_create(int val) {
    Node *n = malloc(sizeof(Node));
    n->val  = val;
    n->next = NULL;
    return n;
}

// Insert at front — O(1)
void list_push_front(List *l, int val) {
    Node *n = node_create(val);
    n->next = l->head;
    l->head = n;
    l->length++;
}

// Insert at back — O(n)
void list_push_back(List *l, int val) {
    Node *n = node_create(val);
    if (!l->head) { l->head = n; l->length++; return; }
    Node *cur = l->head;
    while (cur->next) cur = cur->next;
    cur->next = n;
    l->length++;
}

// Remove from front — O(1)
bool list_pop_front(List *l, int *out) {
    if (!l->head) return false;
    Node *old = l->head;
    *out      = old->val;
    l->head   = old->next;
    free(old);
    l->length--;
    return true;
}

// Insert after the first node with value `after` — O(n)
bool list_insert_after(List *l, int after, int val) {
    Node *cur = l->head;
    while (cur) {
        if (cur->val == after) {
            Node *n  = node_create(val);
            n->next  = cur->next;
            cur->next = n;
            l->length++;
            return true;
        }
        cur = cur->next;
    }
    return false;
}

// Delete first node with matching value — O(n)
bool list_delete(List *l, int val) {
    Node **cur = &l->head;
    while (*cur) {
        if ((*cur)->val == val) {
            Node *old = *cur;
            *cur      = old->next;
            free(old);
            l->length--;
            return true;
        }
        cur = &(*cur)->next;
    }
    return false;
}

// Search — returns pointer to first matching node, NULL if not found — O(n)
Node *list_find(List *l, int val) {
    Node *cur = l->head;
    while (cur) {
        if (cur->val == val) return cur;
        cur = cur->next;
    }
    return NULL;
}

// Reverse in place — O(n)
void list_reverse(List *l) {
    Node *prev = NULL;
    Node *cur  = l->head;
    while (cur) {
        Node *next = cur->next;
        cur->next  = prev;
        prev       = cur;
        cur        = next;
    }
    l->head = prev;
}

void list_free(List *l) {
    Node *cur = l->head;
    while (cur) { Node *next = cur->next; free(cur); cur = next; }
    l->head   = NULL;
    l->length = 0;
}

void list_print(List *l) {
    Node *cur = l->head;
    while (cur) { printf("%d", cur->val); if (cur->next) printf(" -> "); cur = cur->next; }
    printf("  (length=%zu)\n", l->length);
}


int main(void) {
    List l = list_init();

    printf("=== Push front ===\n");
    list_push_front(&l, 30);
    list_push_front(&l, 20);
    list_push_front(&l, 10);
    list_print(&l);   // 10 -> 20 -> 30

    printf("\n=== Push back ===\n");
    list_push_back(&l, 40);
    list_push_back(&l, 50);
    list_print(&l);   // 10 -> 20 -> 30 -> 40 -> 50

    printf("\n=== Insert after 30 ===\n");
    list_insert_after(&l, 30, 35);
    list_print(&l);   // 10 -> 20 -> 30 -> 35 -> 40 -> 50

    printf("\n=== Search ===\n");
    Node *found = list_find(&l, 35);
    printf("find(35): %s\n", found ? "found" : "not found");
    printf("find(99): %s\n", list_find(&l, 99) ? "found" : "not found");

    printf("\n=== Delete ===\n");
    list_delete(&l, 10);   // head
    list_delete(&l, 35);   // middle
    list_delete(&l, 50);   // tail
    list_print(&l);        // 20 -> 30 -> 40

    printf("\n=== Pop front ===\n");
    int val;
    list_pop_front(&l, &val);
    printf("popped %d  |  ", val);
    list_print(&l);

    printf("\n=== Reverse ===\n");
    list_reverse(&l);
    list_print(&l);

    list_free(&l);
    return 0;
}
