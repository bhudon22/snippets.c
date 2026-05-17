#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// =============================================================================
// HASH TABLE — Open Addressing (Linear Probing)
//
// All entries live in a flat array — no linked lists, no heap allocations
// beyond the table itself. On collision, probe forward through the array
// until an empty slot is found.
//
// Strengths: better cache locality than chaining (flat array), simpler memory
//            management, faster in practice at low-to-medium load.
// Weaknesses: performance degrades sharply above ~70% load (clustering).
//             Deletion is tricky — must use a DELETED tombstone rather than
//             EMPTY, otherwise probes stop too early and lose entries.
//
// Key type: char[64] (fixed-width, no heap)   Value type: int
// Hash function: djb2
// =============================================================================

#define CAPACITY 16
#define MAX_KEY  64

typedef enum { EMPTY, OCCUPIED, DELETED } SlotState;

typedef struct {
    char      key[MAX_KEY];
    int       val;
    SlotState state;
} Slot;

typedef struct {
    Slot   slots[CAPACITY];
    size_t length;
} HashTable;

static size_t hash(const char *key) {
    size_t h = 5381;
    while (*key) h = ((h << 5) + h) + (unsigned char)*key++;
    return h % CAPACITY;
}

HashTable ht_init(void) {
    HashTable ht = {0};
    for (int i = 0; i < CAPACITY; i++) ht.slots[i].state = EMPTY;
    return ht;
}

// Insert or update — probes forward on collision — O(1) average
bool ht_set(HashTable *ht, const char *key, int val) {
    size_t idx   = hash(key);
    size_t first_deleted = (size_t)-1;

    for (size_t i = 0; i < CAPACITY; i++) {
        size_t slot = (idx + i) % CAPACITY;
        Slot  *s    = &ht->slots[slot];

        if (s->state == OCCUPIED && strcmp(s->key, key) == 0) {
            s->val = val;   // update existing key
            return true;
        }
        if (s->state == DELETED && first_deleted == (size_t)-1) {
            first_deleted = slot;   // remember first tombstone — reuse it
        }
        if (s->state == EMPTY) {
            size_t target = (first_deleted != (size_t)-1) ? first_deleted : slot;
            strncpy(ht->slots[target].key, key, MAX_KEY - 1);
            ht->slots[target].val   = val;
            ht->slots[target].state = OCCUPIED;
            ht->length++;
            return true;
        }
    }
    return false;   // table full
}

// Get — O(1) average
bool ht_get(HashTable *ht, const char *key, int *out) {
    size_t idx = hash(key);
    for (size_t i = 0; i < CAPACITY; i++) {
        size_t slot = (idx + i) % CAPACITY;
        Slot  *s    = &ht->slots[slot];
        if (s->state == EMPTY) break;   // probe chain ends here
        if (s->state == OCCUPIED && strcmp(s->key, key) == 0) {
            *out = s->val;
            return true;
        }
    }
    return false;
}

// Delete — marks slot as DELETED (tombstone) so probe chains stay intact
bool ht_delete(HashTable *ht, const char *key) {
    size_t idx = hash(key);
    for (size_t i = 0; i < CAPACITY; i++) {
        size_t slot = (idx + i) % CAPACITY;
        Slot  *s    = &ht->slots[slot];
        if (s->state == EMPTY) break;
        if (s->state == OCCUPIED && strcmp(s->key, key) == 0) {
            s->state = DELETED;   // tombstone — NOT empty, so probes continue past it
            ht->length--;
            return true;
        }
    }
    return false;
}

void ht_print(HashTable *ht) {
    for (int i = 0; i < CAPACITY; i++) {
        Slot *s = &ht->slots[i];
        if      (s->state == OCCUPIED) printf("  [%2d] \"%s\": %d\n", i, s->key, s->val);
        else if (s->state == DELETED)  printf("  [%2d] <deleted>\n", i);
        else                           printf("  [%2d] -\n", i);
    }
    printf("  %zu entries, load=%.0f%%\n", ht->length, 100.0 * ht->length / CAPACITY);
}


int main(void) {
    HashTable ht = ht_init();

    printf("=== Insert ===\n");
    ht_set(&ht, "apple",      5);
    ht_set(&ht, "banana",     3);
    ht_set(&ht, "cherry",     8);
    ht_set(&ht, "date",       1);
    ht_set(&ht, "elderberry", 12);
    ht_set(&ht, "fig",        7);
    ht_print(&ht);

    printf("\n=== Get ===\n");
    int val;
    ht_get(&ht, "cherry", &val); printf("cherry: %d\n", val);
    printf("mango: %s\n", ht_get(&ht, "mango", &val) ? "found" : "not found");

    printf("\n=== Update ===\n");
    ht_set(&ht, "apple", 99);
    ht_get(&ht, "apple", &val);
    printf("apple (updated): %d\n", val);

    printf("\n=== Delete and tombstone ===\n");
    ht_delete(&ht, "banana");
    ht_print(&ht);   // banana slot shows <deleted>, not empty

    printf("\n=== Entries after deletion still findable ===\n");
    // Keys that probe past the deleted slot must still be found
    ht_get(&ht, "cherry", &val); printf("cherry (past tombstone): %d\n", val);
    printf("banana after delete: %s\n", ht_get(&ht, "banana", &val) ? "found" : "not found");

    printf("\n=== Reuse tombstone slot on insert ===\n");
    ht_set(&ht, "grape", 42);
    ht_print(&ht);   // grape may land in the old banana slot

    return 0;
}
