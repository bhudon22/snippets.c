# Stacks & Queues

Five classic data structures built on fixed-size arrays in C. One file per structure, self-contained, no dependencies.

---

## [stack.c](./stack.c) — LIFO Stack

Last In, First Out. Push and pop from the top only. Think of a stack of plates.

```c
Stack s = stack_init();
stack_push(&s, 10);
stack_push(&s, 20);
stack_push(&s, 30);

int val;
stack_pop(&s, &val);   // => 30
stack_pop(&s, &val);   // => 20
```

| Function | Description |
|---|---|
| `stack_push(&s, val)` | Push onto the top |
| `stack_pop(&s, &out)` | Remove and return top item |
| `stack_peek(&s, &out)` | Read top without removing |
| `stack_is_empty(&s)` | True if no items |
| `stack_is_full(&s)` | True if at capacity |

---

## [queue.c](./queue.c) — FIFO Queue

First In, First Out. Enqueue at the back, dequeue from the front. Think of a checkout line. Implemented as a circular array — both operations are O(1) with no element shifting.

```c
Queue q = queue_init();
queue_enqueue(&q, 10);
queue_enqueue(&q, 20);
queue_enqueue(&q, 30);

int val;
queue_dequeue(&q, &val);   // => 10
queue_dequeue(&q, &val);   // => 20
```

| Function | Description |
|---|---|
| `queue_enqueue(&q, val)` | Add to the back |
| `queue_dequeue(&q, &out)` | Remove and return front item |
| `queue_peek(&q, &out)` | Read front without removing |
| `queue_is_empty(&q)` | True if no items |
| `queue_is_full(&q)` | True if at capacity |

---

## [deque.c](./deque.c) — Double-Ended Queue

Push and pop from either end. A superset of Stack and Queue — you can use it as either, or both at once. Also a circular array under the hood.

```c
Deque d = deque_init();
deque_push_back(&d, 20);
deque_push_front(&d, 10);   // front -> [10] [20] <- back
deque_push_back(&d, 30);    // front -> [10] [20] [30] <- back

int val;
deque_pop_front(&d, &val);  // => 10
deque_pop_back(&d, &val);   // => 30
```

| Function | Description |
|---|---|
| `deque_push_front(&d, val)` | Add to the front |
| `deque_push_back(&d, val)` | Add to the back |
| `deque_pop_front(&d, &out)` | Remove and return front item |
| `deque_pop_back(&d, &out)` | Remove and return back item |
| `deque_peek_front/back` | Read without removing |

---

## [circular_buffer.c](./circular_buffer.c) — Ring Buffer

Fixed-size buffer where the write head chases the read head in a circle. When full, **writing overwrites the oldest data automatically** — the writer never blocks. All operations O(1).

Classic uses: audio streams, serial/network data, sensor readings — anywhere you want a sliding window of the most recent N items.

```c
CircularBuffer cb = cb_init();

// Fill it up, then keep writing — oldest items silently drop
for (int i = 0; i < 12; i++) cb_write(&cb, i);

int val;
cb_read(&cb, &val);   // reads oldest surviving item
```

| Function | Description |
|---|---|
| `cb_write(&cb, val)` | Write — overwrites oldest if full |
| `cb_read(&cb, &out)` | Read and consume oldest item |
| `cb_peek(&cb, &out)` | Read oldest without consuming |
| `cb_is_empty/full` | State checks |

Key difference from Queue: a queue refuses to enqueue when full. A circular buffer silently drops the oldest item to make room.

---

## [priority_queue.c](./priority_queue.c) — Priority Queue (Min-Heap)

Items are dequeued by priority, not insertion order. The smallest value always comes out first. Push and pop are both O(log n); peek is O(1).

Backed by a binary heap in a plain array:
- Parent at index `i` → children at `2i+1` and `2i+2`
- Push: append at end, sift UP
- Pop: swap root with last, shrink, sift DOWN

```c
PriorityQueue pq = pq_init();
pq_push(&pq, 40);
pq_push(&pq, 10);
pq_push(&pq, 70);
pq_push(&pq, 20);

int val;
pq_pop(&pq, &val);   // => 10  (always the minimum)
pq_pop(&pq, &val);   // => 20
pq_pop(&pq, &val);   // => 40
```

| Function | Description |
|---|---|
| `pq_push(&pq, val)` | Insert — O(log n) |
| `pq_pop(&pq, &out)` | Remove and return minimum — O(log n) |
| `pq_peek(&pq, &out)` | Read minimum without removing — O(1) |

Swap the `<` comparator in `sift_down` / `sift_up` to get a max-heap (largest first).

---

## Complexity summary

| Structure | Push/Enqueue | Pop/Dequeue | Peek | Notes |
|---|---|---|---|---|
| Stack | O(1) | O(1) | O(1) | |
| Queue | O(1) | O(1) | O(1) | Circular array |
| Deque | O(1) | O(1) | O(1) | Circular array |
| Circular Buffer | O(1) | O(1) | O(1) | Overwrites on full |
| Priority Queue | O(log n) | O(log n) | O(1) | Binary heap |
