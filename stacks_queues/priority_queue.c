#include <stdio.h>
#include <stdbool.h>

// =============================================================================
// PRIORITY QUEUE (Min-Heap)
//
// Items are dequeued by priority, not insertion order. The item with the
// lowest value always comes out first (min-heap). Swap the comparator for
// a max-heap (highest first).
//
// Backed by a binary heap stored in a plain array. The heap property:
//   parent <= both children
//
// Array layout — for item at index i:
//   parent:       (i - 1) / 2
//   left child:   2*i + 1
//   right child:  2*i + 2
//
// Push: append at the end, then sift UP  — O(log n)
// Pop:  swap root with last, shrink, sift DOWN — O(log n)
// Peek: read index 0 — O(1)
// =============================================================================

#define MAX 16

typedef struct {
    int    data[MAX];
    size_t length;
} PriorityQueue;

PriorityQueue pq_init(void)           { return (PriorityQueue){0}; }
bool          pq_is_empty(PriorityQueue *pq) { return pq->length == 0; }
bool          pq_is_full(PriorityQueue *pq)  { return pq->length == MAX; }

static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

// Sift UP — restore heap property after inserting at the end
static void sift_up(PriorityQueue *pq, size_t i) {
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (pq->data[parent] <= pq->data[i]) break;
        swap(&pq->data[parent], &pq->data[i]);
        i = parent;
    }
}

// Sift DOWN — restore heap property after removing the root
static void sift_down(PriorityQueue *pq, size_t i) {
    while (true) {
        size_t smallest = i;
        size_t left     = 2 * i + 1;
        size_t right    = 2 * i + 2;

        if (left  < pq->length && pq->data[left]  < pq->data[smallest]) smallest = left;
        if (right < pq->length && pq->data[right] < pq->data[smallest]) smallest = right;

        if (smallest == i) break;
        swap(&pq->data[i], &pq->data[smallest]);
        i = smallest;
    }
}

bool pq_push(PriorityQueue *pq, int val) {
    if (pq_is_full(pq)) return false;
    pq->data[pq->length++] = val;
    sift_up(pq, pq->length - 1);
    return true;
}

bool pq_pop(PriorityQueue *pq, int *out) {
    if (pq_is_empty(pq)) return false;
    *out = pq->data[0];
    pq->data[0] = pq->data[--pq->length];  // move last item to root
    sift_down(pq, 0);
    return true;
}

bool pq_peek(PriorityQueue *pq, int *out) {
    if (pq_is_empty(pq)) return false;
    *out = pq->data[0];
    return true;
}

void pq_print(PriorityQueue *pq) {
    printf("heap: ");
    for (size_t i = 0; i < pq->length; i++) printf("[%d] ", pq->data[i]);
    printf("  min=%d  (size=%zu)\n", pq->data[0], pq->length);
}


int main(void) {
    PriorityQueue pq = pq_init();

    printf("=== Push (inserted out of order) ===\n");
    int vals[] = { 40, 10, 70, 30, 50, 20, 60 };
    for (int i = 0; i < 7; i++) {
        pq_push(&pq, vals[i]);
        pq_print(&pq);
    }

    printf("\n=== Peek ===\n");
    int top;
    pq_peek(&pq, &top);
    printf("min = %d\n", top);

    printf("\n=== Pop (always returns the minimum) ===\n");
    int val;
    while (pq_pop(&pq, &val)) printf("popped %d\n", val);

    printf("\n=== Task scheduler simulation ===\n");
    // Lower number = higher priority
    PriorityQueue tasks = pq_init();
    pq_push(&tasks, 3);   // low priority
    pq_push(&tasks, 1);   // critical
    pq_push(&tasks, 5);   // background
    pq_push(&tasks, 2);   // high priority
    pq_push(&tasks, 4);   // normal

    printf("processing tasks by priority:\n");
    int priority;
    while (pq_pop(&tasks, &priority))
        printf("  task priority %d\n", priority);

    return 0;
}
