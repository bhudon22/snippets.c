#include <stdio.h>
#include <stdbool.h>

// =============================================================================
// QUEUE — FIFO (First In, First Out)
//
// Items enter at the back and leave from the front.
// Think of a checkout line — first in, first served.
//
// Implemented as a circular array so both enqueue and dequeue are O(1)
// without shifting elements. Head and tail indices wrap around using modulo.
// =============================================================================

#define MAX 8

typedef struct {
    int  data[MAX];
    int  head;      // index of the front item (next to dequeue)
    int  tail;      // index of the next free slot (where next enqueue goes)
    int  length;
} Queue;

Queue queue_init(void)         { return (Queue){0}; }
bool  queue_is_empty(Queue *q) { return q->length == 0; }
bool  queue_is_full(Queue *q)  { return q->length == MAX; }

bool queue_enqueue(Queue *q, int val) {
    if (queue_is_full(q)) return false;
    q->data[q->tail] = val;
    q->tail = (q->tail + 1) % MAX;   // wrap around
    q->length++;
    return true;
}

bool queue_dequeue(Queue *q, int *out) {
    if (queue_is_empty(q)) return false;
    *out = q->data[q->head];
    q->head = (q->head + 1) % MAX;   // wrap around
    q->length--;
    return true;
}

bool queue_peek(Queue *q, int *out) {
    if (queue_is_empty(q)) return false;
    *out = q->data[q->head];
    return true;
}

void queue_print(Queue *q) {
    printf("front -> ");
    for (int i = 0; i < q->length; i++)
        printf("[%d] ", q->data[(q->head + i) % MAX]);
    printf("<- back  (size=%d)\n", q->length);
}


int main(void) {
    Queue q = queue_init();

    printf("=== Enqueue ===\n");
    int vals[] = { 10, 20, 30, 40, 50 };
    for (int i = 0; i < 5; i++) {
        queue_enqueue(&q, vals[i]);
        queue_print(&q);
    }

    printf("\n=== Peek ===\n");
    int front;
    queue_peek(&q, &front);
    printf("front = %d\n", front);

    printf("\n=== Dequeue ===\n");
    int val;
    while (queue_dequeue(&q, &val)) {
        printf("dequeued %d  |  ", val);
        queue_print(&q);
    }

    printf("\n=== Wrap-around (circular behaviour) ===\n");
    // Fill, partially drain, then fill again — indices wrap past MAX
    for (int i = 0; i < 6; i++) queue_enqueue(&q, i * 5);
    for (int i = 0; i < 3; i++) { queue_dequeue(&q, &val); printf("dequeued %d\n", val); }
    for (int i = 0; i < 5; i++) queue_enqueue(&q, 100 + i);
    queue_print(&q);

    printf("\n=== Overflow guard ===\n");
    Queue q2 = queue_init();
    for (int i = 0; i < MAX + 2; i++) {
        bool ok = queue_enqueue(&q2, i);
        if (!ok) printf("enqueue %d failed — queue full\n", i);
    }

    return 0;
}
