# Hash Table

Two implementations of the same interface, using different collision strategies. Both use the djb2 hash function for string keys.

---

## [chaining.c](./chaining.c) — Separate Chaining

Each bucket holds a linked list of entries. Collisions are handled by appending to that list. Load factor can exceed 1.0 without breaking.

```
bucket[2] -> "cherry":8 -> NULL
bucket[6] -> "banana":3 -> NULL
bucket[7] -> "apple":5 -> NULL
              ...
```

```c
HashTable ht = ht_init();
ht_set(&ht, "apple",  5);
ht_set(&ht, "banana", 3);
ht_set(&ht, "apple",  9);   // update existing key

int val;
ht_get(&ht, "apple", &val); // => 9
ht_delete(&ht, "banana");
ht_free(&ht);
```

| Function | Description | Complexity |
|---|---|---|
| `ht_set(&ht, key, val)` | Insert or update | O(1) avg |
| `ht_get(&ht, key, &out)` | Lookup | O(1) avg |
| `ht_delete(&ht, key)` | Remove | O(1) avg |
| `ht_free(&ht)` | Free all entries | O(n) |

---

## [open_addressing.c](./open_addressing.c) — Linear Probing

All entries live in a flat array — no linked lists, no heap allocations per entry. On collision, probe forward until an empty slot is found.

```
slot[ 2] "cherry": 8
slot[ 3] "date":   1
slot[ 6] <deleted>       ← tombstone, not empty
slot[ 7] "apple":  99
slot[11] "fig":    7
```

```c
HashTable ht = ht_init();
ht_set(&ht, "apple",  5);
ht_set(&ht, "banana", 3);

int val;
ht_get(&ht, "apple", &val);  // => 5

ht_delete(&ht, "banana");    // marks slot DELETED (tombstone)
ht_set(&ht, "grape", 42);    // may reuse the tombstone slot
```

| Function | Description | Complexity |
|---|---|---|
| `ht_set(&ht, key, val)` | Insert or update | O(1) avg |
| `ht_get(&ht, key, &out)` | Lookup | O(1) avg |
| `ht_delete(&ht, key)` | Mark as DELETED tombstone | O(1) avg |

**Why tombstones?** Marking a deleted slot as EMPTY would break probe chains — a later lookup would stop at the empty slot and miss entries that were inserted past the deleted one. The DELETED state tells probes to keep going, while inserts can reuse the slot.

---

## Chaining vs Open Addressing

| | Chaining | Open Addressing |
|---|---|---|
| Memory layout | Linked list per bucket | Flat array |
| Cache locality | Poor (scattered nodes) | Good (contiguous slots) |
| Load factor > 1.0 | Works fine | Breaks down |
| Best performance at | Any load | Load < 70% |
| Deletion | Simple (unlink node) | Needs tombstone |
| Memory overhead | Extra pointer per entry | Fixed-size key (no heap per entry) |
| Use when | High load or many deletes | Low load, cache perf matters |

---

## Hash function — djb2

Both files use djb2, a simple and effective string hash:

```c
size_t h = 5381;
while (*key) h = ((h << 5) + h) + (unsigned char)*key++;
return h % CAPACITY;
```

`(h << 5) + h` is `h * 33` — the magic constant that gives djb2 good distribution in practice.
