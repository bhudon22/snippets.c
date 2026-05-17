#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// =============================================================================
// HASH TABLE — Separate Chaining
//
// Each bucket holds a linked list of entries. Collisions are handled by
// appending to that list. Load factor can exceed 1.0 without breaking,
// though performance degrades as chains get longer.
//
// Strengths: simple, handles high load factors gracefully, deletion is easy.
// Weaknesses: extra pointer per entry, poor cache locality (linked list nodes
//             scattered in heap memory).
//
// Key type: const char* (string)   Value type: int
// Hash function: djb2
// =============================================================================

#define CAPACITY 16

typedef struct Entry {
    char        *key;
    int          val;
    struct Entry *next;
} Entry;

typedef struct {
    Entry  *buckets[CAPACITY];
    size_t  length;     // number of key-value pairs stored
} HashTable;

// djb2 — simple, fast, good distribution for string keys
static size_t hash(const char *key) {
    size_t h = 5381;
    while (*key) h = ((h << 5) + h) + (unsigned char)*key++;
    return h % CAPACITY;
}

HashTable ht_init(void) { return (HashTable){0}; }

// Insert or update — O(n/CAPACITY) average
void ht_set(HashTable *ht, const char *key, int val) {
    size_t idx = hash(key);
    Entry *cur = ht->buckets[idx];

    // Update if key already exists
    while (cur) {
        if (strcmp(cur->key, key) == 0) { cur->val = val; return; }
        cur = cur->next;
    }

    // Insert new entry at head of chain
    Entry *e  = malloc(sizeof(Entry));
    e->key    = strdup(key);
    e->val    = val;
    e->next   = ht->buckets[idx];
    ht->buckets[idx] = e;
    ht->length++;
}

// Get — returns true and sets *out if found — O(n/CAPACITY) average
bool ht_get(HashTable *ht, const char *key, int *out) {
    Entry *cur = ht->buckets[hash(key)];
    while (cur) {
        if (strcmp(cur->key, key) == 0) { *out = cur->val; return true; }
        cur = cur->next;
    }
    return false;
}

// Delete — O(n/CAPACITY) average
bool ht_delete(HashTable *ht, const char *key) {
    size_t  idx = hash(key);
    Entry **cur = &ht->buckets[idx];
    while (*cur) {
        if (strcmp((*cur)->key, key) == 0) {
            Entry *old = *cur;
            *cur       = old->next;
            free(old->key);
            free(old);
            ht->length--;
            return true;
        }
        cur = &(*cur)->next;
    }
    return false;
}

void ht_free(HashTable *ht) {
    for (int i = 0; i < CAPACITY; i++) {
        Entry *cur = ht->buckets[i];
        while (cur) { Entry *next = cur->next; free(cur->key); free(cur); cur = next; }
        ht->buckets[i] = NULL;
    }
    ht->length = 0;
}

void ht_print(HashTable *ht) {
    for (int i = 0; i < CAPACITY; i++) {
        if (!ht->buckets[i]) continue;
        printf("  [%2d] ", i);
        Entry *cur = ht->buckets[i];
        while (cur) {
            printf("\"%s\":%d", cur->key, cur->val);
            if (cur->next) printf(" -> ");
            cur = cur->next;
        }
        printf("\n");
    }
    printf("  %zu entries, load=%.2f\n", ht->length, (double)ht->length / CAPACITY);
}


int main(void) {
    HashTable ht = ht_init();

    printf("=== Insert ===\n");
    ht_set(&ht, "apple",  5);
    ht_set(&ht, "banana", 3);
    ht_set(&ht, "cherry", 8);
    ht_set(&ht, "date",   1);
    ht_set(&ht, "elderberry", 12);
    ht_set(&ht, "fig",    7);
    ht_print(&ht);

    printf("\n=== Get ===\n");
    int val;
    ht_get(&ht, "cherry", &val); printf("cherry: %d\n", val);
    printf("mango: %s\n", ht_get(&ht, "mango", &val) ? "found" : "not found");

    printf("\n=== Update ===\n");
    ht_set(&ht, "apple", 99);
    ht_get(&ht, "apple", &val);
    printf("apple (updated): %d\n", val);

    printf("\n=== Delete ===\n");
    ht_delete(&ht, "banana");
    printf("banana after delete: %s\n", ht_get(&ht, "banana", &val) ? "found" : "not found");
    ht_print(&ht);

    printf("\n=== Collision demo — words that hash to the same bucket ===\n");
    HashTable ht2 = ht_init();
    // Insert enough entries that some buckets will have chains
    const char *words[] = { "a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r" };
    for (int i = 0; i < 18; i++) ht_set(&ht2, words[i], i);
    ht_print(&ht2);
    ht_free(&ht2);

    ht_free(&ht);
    return 0;
}
