#include <stdio.h>
#include <stdbool.h>

// =============================================================================
// CIRCULAR BUFFER (Ring Buffer)
//
// Fixed-size buffer where the write head chases the read head in a circle.
// When full, writing overwrites the oldest data automatically.
//
// Classic use cases: audio streams, serial/network data, sensor readings —
// anywhere you want a sliding window of the most recent N items and don't
// care about data older than the buffer capacity.
//
// Compared to the Queue: a queue refuses to overwrite (returns false when full).
// A circular buffer silently drops the oldest item to make room — the writer
// never blocks.
//
// All operations O(1).
// =============================================================================

#define CAPACITY 8

typedef struct {
    int  data[CAPACITY];
    int  head;      // read position (oldest item)
    int  tail;      // write position (next slot to write)
    int  length;
} CircularBuffer;

CircularBuffer cb_init(void)           { return (CircularBuffer){0}; }
bool           cb_is_empty(CircularBuffer *cb) { return cb->length == 0; }
bool           cb_is_full(CircularBuffer *cb)  { return cb->length == CAPACITY; }

// Write — always succeeds. If full, overwrites the oldest item.
void cb_write(CircularBuffer *cb, int val) {
    if (cb_is_full(cb)) {
        cb->head = (cb->head + 1) % CAPACITY;  // drop oldest
        cb->length--;
    }
    cb->data[cb->tail] = val;
    cb->tail = (cb->tail + 1) % CAPACITY;
    cb->length++;
}

// Read — returns false if empty
bool cb_read(CircularBuffer *cb, int *out) {
    if (cb_is_empty(cb)) return false;
    *out     = cb->data[cb->head];
    cb->head = (cb->head + 1) % CAPACITY;
    cb->length--;
    return true;
}

// Peek at the oldest item without consuming it
bool cb_peek(CircularBuffer *cb, int *out) {
    if (cb_is_empty(cb)) return false;
    *out = cb->data[cb->head];
    return true;
}

void cb_print(CircularBuffer *cb) {
    printf("oldest -> ");
    for (int i = 0; i < cb->length; i++)
        printf("[%d] ", cb->data[(cb->head + i) % CAPACITY]);
    printf("<- newest  (size=%d/%d)\n", cb->length, CAPACITY);
}


int main(void) {
    CircularBuffer cb = cb_init();

    printf("=== Write until full ===\n");
    for (int i = 1; i <= CAPACITY; i++) {
        cb_write(&cb, i * 10);
        cb_print(&cb);
    }

    printf("\n=== Overwrite behaviour (write past full) ===\n");
    // Writing 3 more — oldest 3 items are silently dropped
    for (int i = 1; i <= 3; i++) {
        printf("writing %d — ", 100 + i);
        cb_write(&cb, 100 + i);
        cb_print(&cb);
    }

    printf("\n=== Read ===\n");
    int val;
    for (int i = 0; i < 4; i++) {
        cb_read(&cb, &val);
        printf("read %d  |  ", val);
        cb_print(&cb);
    }

    printf("\n=== Continuous stream simulation ===\n");
    // Write and read interleaved — models a producer/consumer
    CircularBuffer stream = cb_init();
    for (int frame = 0; frame < 12; frame++) {
        cb_write(&stream, frame);
        if (frame % 3 == 2) {           // consumer reads every 3 frames
            cb_read(&stream, &val);
            printf("frame %2d: read %d  |  ", frame, val);
            cb_print(&stream);
        }
    }

    return 0;
}
