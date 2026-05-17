#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// =============================================================================
// All three bytesbeneath.com modules wired together:
//   1. Allocator interface     — shared glue between all three
//   2. Arena allocator         — bytesbeneath.com/p/the-arena-custom-memory-allocators
//   3. Dynamic arrays          — bytesbeneath.com/p/dynamic-arrays-in-c
//   4. Custom strings          — bytesbeneath.com/p/custom-strings-in-c
//
// Bugs fixed from the articles:
//   - array_init called alloc with 1 arg instead of 2 (missing context)
//   - str_join used s->len (first string length) instead of h->length (array count)
//   - str_clone allocated len bytes, not len+1 (missing null terminator)
//   - Allocator passed by value in arena article, by pointer in string/array articles
//     — unified to always pass Allocator* in all functions
// =============================================================================


// =============================================================================
// 1. ALLOCATOR INTERFACE
//
// A function-pointer pair + opaque context. Lets all functions below accept
// any backing allocator (arena, malloc, pool) without caring which it is.
// =============================================================================
typedef struct {
    void *(*alloc)(size_t size, void *context);
    void  (*free) (size_t size, void *ptr, void *context);
    void  *context;
} Allocator;


// =============================================================================
// 2. ARENA
// =============================================================================
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))
#define is_power_of_two(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

typedef struct {
    void   *base;
    size_t  size;
    size_t  offset;
    size_t  committed;
} Arena;

static uintptr_t align_forward(uintptr_t ptr, size_t alignment) {
    uintptr_t modulo = ptr & ((uintptr_t)alignment - 1);
    if (modulo) ptr += alignment - modulo;
    return ptr;
}

static void *arena_alloc_aligned(Arena *a, size_t size, size_t alignment) {
    if (!is_power_of_two(alignment)) return NULL;
    uintptr_t curr    = (uintptr_t)a->base + a->offset;
    uintptr_t aligned = align_forward(curr, alignment);
    size_t new_offset = (aligned - (uintptr_t)a->base) + size;
    if (new_offset > a->size) return NULL;
    a->committed += size;
    a->offset     = new_offset;
    return (void *)aligned;
}

static void *arena_alloc(size_t size, void *context) {
    if (!size) return NULL;
    return arena_alloc_aligned((Arena *)context, size, DEFAULT_ALIGNMENT);
}

static void arena_free_fn(size_t size, void *ptr, void *context) {
    (void)size; (void)ptr; (void)context;  // intentional no-op
}

Arena arena_init(void *buffer, size_t size) {
    return (Arena){ .base = buffer, .size = size };
}

void arena_free_all(Arena *a) {
    a->offset    = 0;
    a->committed = 0;
}

// Returns an Allocator backed by this arena
Allocator arena_allocator(Arena *a) {
    return (Allocator){ arena_alloc, arena_free_fn, a };
}


// =============================================================================
// 3. DYNAMIC ARRAY
//
// Header trick (Sean Barrett / stb-style): allocate one block containing the
// Array_Header followed immediately by the array data. Return a pointer to the
// data — the caller gets a plain C array they can index with []. The header is
// recovered by subtracting 1 from that pointer (array_header macro).
//
// Note: when arena-backed, reallocation "wastes" the old block (free is a
// no-op). Pre-size the arena generously or use a heap allocator if the array
// will grow a lot.
// =============================================================================
typedef struct {
    size_t     length;
    size_t     capacity;
    size_t     _pad;     // pads struct to 32 bytes for 16-byte aligned data
    Allocator *a;
} Array_Header;

#define ARRAY_INITIAL_CAPACITY 16

#define array(T, a)        ((T *)array_init(sizeof(T), ARRAY_INITIAL_CAPACITY, (a)))
#define array_header(a)    ((Array_Header *)(a) - 1)
#define array_length(a)    (array_header(a)->length)
#define array_capacity(a)  (array_header(a)->capacity)
#define array_pop_back(a)  (array_header(a)->length -= 1)

// Appends v to array a. Grows if needed. Returns pointer to inserted element.
#define array_append(a, v) (                                    \
    (a) = array_ensure_capacity((a), 1, sizeof(*(a))),          \
    (a)[array_header(a)->length] = (v),                         \
    &(a)[array_header(a)->length++])

// Swap-remove: replaces element i with the last element (O(1), order not preserved)
#define array_remove(a, i) do {                                 \
    Array_Header *_h = array_header(a);                         \
    if ((i) == _h->length - 1) {                                \
        _h->length -= 1;                                        \
    } else if (_h->length > 1) {                                \
        (a)[i] = (a)[_h->length - 1];                          \
        _h->length -= 1;                                        \
    }                                                           \
} while (0)

void *array_init(size_t item_size, size_t capacity, Allocator *a) {
    size_t size    = sizeof(Array_Header) + item_size * capacity;
    Array_Header *h = a->alloc(size, a->context);   // FIX: original passed 1 arg
    if (!h) return NULL;
    h->capacity = capacity;
    h->length   = 0;
    h->a        = a;
    return h + 1;
}

void *array_ensure_capacity(void *a, size_t item_count, size_t item_size) {
    Array_Header *h       = array_header(a);
    size_t        desired = h->length + item_count;

    if (h->capacity >= desired) return (Array_Header *)h + 1;

    size_t new_cap  = h->capacity * 2;
    while (new_cap < desired) new_cap *= 2;

    size_t        new_size = sizeof(Array_Header) + new_cap * item_size;
    Array_Header *new_h    = h->a->alloc(new_size, h->a->context);

    if (new_h) {
        size_t old_size = sizeof(*h) + h->length * item_size;
        memcpy(new_h, h, old_size);
        if (h->a->free) h->a->free(old_size, h, h->a->context);
        new_h->capacity = new_cap;
        return new_h + 1;
    }

    return NULL;
}


// =============================================================================
// 4. CUSTOM STRINGS
//
// Stores length alongside the pointer. Keeps the null terminator so .data can
// be passed directly to printf / any C stdlib function.
//
// Two flavours:
//   - Owning strings: allocated via the arena, have their own memory
//   - Views: just a {len, ptr} into existing memory, no allocation, no copy
// =============================================================================
typedef struct {
    size_t  len;
    char   *data;
} String;

// Str("literal") — wrap a string literal in a String without allocating
#define Str(x) (String){ strlen(x), (x) }

String str_init(size_t len, Allocator *a) {
    String s = {
        .len  = len,
        .data = a->alloc(len + 1, a->context),
    };
    if (s.data) s.data[len] = '\0';
    return s;
}

String str_clone(String s, Allocator *a) {
    if (!s.len) return (String){0};
    String r = str_init(s.len, a);   // FIX: str_init allocates len+1 (null byte included)
    memcpy(r.data, s.data, s.len);
    return r;
}

String str_concat(String s1, String s2, Allocator *a) {
    String s = str_init(s1.len + s2.len, a);
    memcpy(s.data,          s1.data, s1.len);
    memcpy(s.data + s1.len, s2.data, s2.len);
    return s;
}

// View: returns a pointer into existing memory, no allocation
String str_view(String s, size_t start, size_t end) {
    if (end < start || end > s.len) return (String){0};
    return (String){ end - start, s.data + start };
}

// Substring: allocates a copy of s[start..end)
String str_substring(String s, size_t start, size_t end, Allocator *a) {
    if (end > s.len || start >= end) return (String){0};
    String r = str_init(end - start, a);
    memcpy(r.data, s.data + start, r.len);
    return r;
}

bool str_equal(String a, String b) {
    if (a.len != b.len) return false;
    return memcmp(a.data, b.data, a.len) == 0;
}

size_t str_index_of(String haystack, String needle) {
    if (needle.len > haystack.len) return (size_t)-1;
    for (size_t i = 0; i <= haystack.len - needle.len; i++) {
        if (memcmp(&haystack.data[i], needle.data, needle.len) == 0) return i;
    }
    return (size_t)-1;
}

bool str_contains(String haystack, String needle) {
    return str_index_of(haystack, needle) != (size_t)-1;
}

// View from the first occurrence of needle to end of string. No allocation.
// Note: not null-terminated at the end — use %.*s to print safely.
String str_substring_view(String haystack, String needle) {
    size_t idx = str_index_of(haystack, needle);
    if (idx == (size_t)-1) return (String){0};
    return (String){ haystack.len - idx, haystack.data + idx };
}

String *str_split(String s, String delim, Allocator *a) {
    String *arr = array(String, a);
    if (delim.len == 0 || delim.len > s.len) {
        String chunk = str_clone(s, a);
        array_append(arr, chunk);
        return arr;
    }
    size_t start = 0;
    for (size_t i = 0; i <= s.len - delim.len; i++) {
        if (memcmp(&s.data[i], delim.data, delim.len) == 0) {
            String chunk = str_substring(s, start, i, a);
            array_append(arr, chunk);
            start = i + delim.len;
            i    += delim.len - 1;
        }
    }
    if (start <= s.len) {
        String chunk = str_substring(s, start, s.len, a);
        array_append(arr, chunk);
    }
    return arr;
}

String str_join(String *s, String joiner, Allocator *a) {
    Array_Header *h = array_header(s);
    if (h->length == 0) return (String){0};

    // FIX: original used s->len (first string's length) as the array count
    size_t total = joiner.len * (h->length - 1);
    for (size_t i = 0; i < h->length; i++) total += s[i].len;

    String r      = str_init(total, a);
    size_t offset = 0;
    for (size_t i = 0; i < h->length; i++) {
        memcpy(r.data + offset, s[i].data, s[i].len);
        offset += s[i].len;
        if (i < h->length - 1) {
            memcpy(r.data + offset, joiner.data, joiner.len);
            offset += joiner.len;
        }
    }
    return r;
}


// =============================================================================
// EXAMPLES
// =============================================================================
int main(void) {
    void   *buffer   = malloc(1024 * 256);     // 256 KB — one malloc for everything
    Arena   arena    = arena_init(buffer, 1024 * 256);
    Allocator al     = arena_allocator(&arena);

    // -------------------------------------------------------------------------
    printf("=== Dynamic Array ===\n");

    int *squares = array(int, &al);
    for (int i = 0; i < 12; i++) array_append(squares, i * i);

    printf("squares:  ");
    for (size_t i = 0; i < array_length(squares); i++) printf("%d ", squares[i]);
    printf("\nlength=%zu  capacity=%zu\n", array_length(squares), array_capacity(squares));

    array_remove(squares, 0);   // swap-remove index 0
    printf("after remove [0]: first element is now %d\n\n", squares[0]);

    // -------------------------------------------------------------------------
    printf("=== String: concat & clone ===\n");

    String a1      = Str("Hello, ");
    String a2      = Str("World!");
    String greeting = str_concat(a1, a2, &al);
    printf("concat:  %s\n", greeting.data);

    String copy = str_clone(greeting, &al);
    printf("clone:   %s  (equal=%s)\n\n", copy.data, str_equal(greeting, copy) ? "yes" : "no");

    // -------------------------------------------------------------------------
    printf("=== String: split & join ===\n");

    String sentence = Str("the quick brown fox jumps over the lazy dog");
    String *words   = str_split(sentence, Str(" "), &al);
    printf("split '%s'\n", sentence.data);
    printf("into %zu words\n", array_length(words));

    String rejoined = str_join(words, Str("-"), &al);
    printf("joined:  %s\n\n", rejoined.data);

    // -------------------------------------------------------------------------
    printf("=== String: search & views ===\n");

    printf("contains 'fox': %s\n", str_contains(sentence, Str("fox")) ? "yes" : "no");
    printf("contains 'cat': %s\n", str_contains(sentence, Str("cat")) ? "yes" : "no");
    printf("index of 'fox': %zu\n", str_index_of(sentence, Str("fox")));

    String view = str_substring_view(sentence, Str("fox"));
    printf("view from 'fox': %.*s\n", (int)view.len, view.data);  // %.*s because not null-terminated at end

    String sub = str_substring(sentence, 4, 9, &al);
    printf("substring [4,9]: %s\n\n", sub.data);

    // -------------------------------------------------------------------------
    printf("=== String array ===\n");

    String *tags = array(String, &al);
    array_append(tags, Str("arena"));
    array_append(tags, Str("strings"));
    array_append(tags, Str("arrays"));
    array_append(tags, Str("C"));

    String tag_line = str_join(tags, Str(", "), &al);
    printf("tags: %s\n\n", tag_line.data);

    // -------------------------------------------------------------------------
    printf("=== Arena stats ===\n");
    printf("used %zu / %zu bytes (%.1f%%)\n",
        arena.offset, arena.size,
        100.0 * (double)arena.offset / (double)arena.size);

    free(buffer);   // one free for everything
    return 0;
}
