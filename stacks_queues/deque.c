#include <stdio.h>
#include <stdbool.h>

// =============================================================================
// DEQUE — Double-Ended Queue (pronounced "deck")
//
// Items can be pushed and popped from either end — front or back.
// A superset of both Stack and Queue: you get LIFO, FIFO, or both at once.
//
// Also implemented as a circular array. Head moves left on push_front,
// right on pop_front. Tail moves right on push_back, left on pop_back.
// All operations are O(1).
// =============================================================================

#define MAX 8

typedef struct {
    int  data[MAX];
    int  head;      // index of the current front item
    int  tail;      // index of the current back item
    int  length;
} Deque;

Deque deque_init(void)         { return (Deque){ .head = 0, .tail = MAX - 1 }; }
bool  deque_is_empty(Deque *d) { return d->length == 0; }
bool  deque_is_full(Deque *d)  { return d->length == MAX; }

bool deque_push_back(Deque *d, int val) {
    if (deque_is_full(d)) return false;
    d->tail = (d->tail + 1) % MAX;
    d->data[d->tail] = val;
    d->length++;
    return true;
}

bool deque_push_front(Deque *d, int val) {
    if (deque_is_full(d)) return false;
    d->head = (d->head - 1 + MAX) % MAX;   // step left, wrap around
    d->data[d->head] = val;
    d->length++;
    return true;
}

bool deque_pop_front(Deque *d, int *out) {
    if (deque_is_empty(d)) return false;
    *out   = d->data[d->head];
    d->head = (d->head + 1) % MAX;
    d->length--;
    return true;
}

bool deque_pop_back(Deque *d, int *out) {
    if (deque_is_empty(d)) return false;
    *out   = d->data[d->tail];
    d->tail = (d->tail - 1 + MAX) % MAX;
    d->length--;
    return true;
}

bool deque_peek_front(Deque *d, int *out) {
    if (deque_is_empty(d)) return false;
    *out = d->data[d->head];
    return true;
}

bool deque_peek_back(Deque *d, int *out) {
    if (deque_is_empty(d)) return false;
    *out = d->data[d->tail];
    return true;
}

void deque_print(Deque *d) {
    printf("front -> ");
    for (int i = 0; i < d->length; i++)
        printf("[%d] ", d->data[(d->head + i) % MAX]);
    printf("<- back  (size=%d)\n", d->length);
}


int main(void) {
    Deque d = deque_init();

    printf("=== Push back ===\n");
    int vals[] = { 10, 20, 30 };
    for (int i = 0; i < 3; i++) {
        deque_push_back(&d, vals[i]);
        deque_print(&d);
    }

    printf("\n=== Push front ===\n");
    int fronts[] = { 5, 1 };
    for (int i = 0; i < 2; i++) {
        deque_push_front(&d, fronts[i]);
        deque_print(&d);
    }

    printf("\n=== Peek ===\n");
    int f, b;
    deque_peek_front(&d, &f);
    deque_peek_back(&d, &b);
    printf("front=%d  back=%d\n", f, b);

    printf("\n=== Pop front ===\n");
    int val;
    deque_pop_front(&d, &val);
    printf("popped front: %d  |  ", val);
    deque_print(&d);

    printf("\n=== Pop back ===\n");
    deque_pop_back(&d, &val);
    printf("popped back: %d  |  ", val);
    deque_print(&d);

    printf("\n=== Used as a LIFO stack (push/pop back) ===\n");
    Deque stack = deque_init();
    for (int i = 1; i <= 4; i++) deque_push_back(&stack, i * 10);
    deque_print(&stack);
    while (deque_pop_back(&stack, &val)) printf("popped %d\n", val);

    printf("\n=== Used as a FIFO queue (push back, pop front) ===\n");
    Deque queue = deque_init();
    for (int i = 1; i <= 4; i++) deque_push_back(&queue, i * 10);
    deque_print(&queue);
    while (deque_pop_front(&queue, &val)) printf("dequeued %d\n", val);

    return 0;
}
