# Linked List

Two implementations: singly and doubly linked. Both use `malloc` per node — the classic approach for learning pointer manipulation.

---

## [singly.c](./singly.c) — Singly Linked List

Each node has a value and a `next` pointer. Traversal is forward only. The list tracks only the head.

```
head -> [10] -> [20] -> [30] -> NULL
```

```c
List l = list_init();
list_push_front(&l, 10);
list_push_back(&l, 20);
list_insert_after(&l, 10, 15);  // 10 -> 15 -> 20

int val;
list_pop_front(&l, &val);       // => 10

list_delete(&l, 15);
list_reverse(&l);
list_free(&l);
```

| Function | Description | Complexity |
|---|---|---|
| `list_push_front(&l, val)` | Insert at head | O(1) |
| `list_push_back(&l, val)` | Insert at tail | O(n) |
| `list_pop_front(&l, &out)` | Remove head | O(1) |
| `list_insert_after(&l, after, val)` | Insert after first match | O(n) |
| `list_delete(&l, val)` | Remove first match | O(n) |
| `list_find(&l, val)` | Search | O(n) |
| `list_reverse(&l)` | Reverse in place | O(n) |
| `list_free(&l)` | Free all nodes | O(n) |

---

## [doubly.c](./doubly.c) — Doubly Linked List

Each node has `prev` and `next` pointers. The list tracks both head and tail, giving O(1) operations at either end. If you have a pointer to a node, removal is O(1) — no search needed.

```
head -> [1] <-> [5] <-> [10] <-> [20] <- tail
```

```c
List l = list_init();
list_push_back(&l, 10);
list_push_front(&l, 5);
list_push_back(&l, 20);

int val;
list_pop_front(&l, &val);   // => 5
list_pop_back(&l, &val);    // => 20

// O(1) removal if you already have the node pointer
Node *n = list_find(&l, 10);
list_unlink(&l, n);

list_free(&l);
```

| Function | Description | Complexity |
|---|---|---|
| `list_push_front(&l, val)` | Insert at head | O(1) |
| `list_push_back(&l, val)` | Insert at tail | O(1) |
| `list_pop_front(&l, &out)` | Remove head | O(1) |
| `list_pop_back(&l, &out)` | Remove tail | O(1) |
| `list_unlink(&l, node)` | Remove a node directly | O(1) |
| `list_delete(&l, val)` | Find and remove first match | O(n) |
| `list_find(&l, val)` | Search | O(n) |
| `list_print_forward/backward` | Traverse either direction | O(n) |

---

## Singly vs Doubly

| | Singly | Doubly |
|---|---|---|
| Memory per node | 1 pointer | 2 pointers |
| Insert/remove at head | O(1) | O(1) |
| Insert/remove at tail | O(n) | O(1) |
| Remove known node | O(n) (need predecessor) | O(1) |
| Backward traversal | No | Yes |
| Use when | Simple, memory-tight | Need tail ops or O(1) remove |
