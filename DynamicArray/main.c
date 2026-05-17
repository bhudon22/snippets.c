#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// DYNAMIC ARRAY — based on bytesbeneath.com/p/dynamic-arrays-in-c
//
// Header trick (Sean Barrett / stb-style): allocate one block containing the
// Array_Header followed immediately by the array data. Return a pointer to the
// data — the caller gets a plain C array they can index with [].
// The header is recovered by subtracting 1 from that pointer (array_header macro).
//
// Bug fixed from article: array_init called alloc with 1 argument — the
// Allocator.alloc signature requires 2 (size + context).
// =============================================================================


// -----------------------------------------------------------------------------
// Allocator interface (shared with Arena and String modules)
// -----------------------------------------------------------------------------
typedef struct {
    void *(*alloc)(size_t size, void *context);
    void  (*free) (size_t size, void *ptr, void *context);
    void  *context;
} Allocator;

// Minimal heap-backed allocator so this file compiles standalone
static void *heap_alloc(size_t size, void *context) {
    (void)context;
    return malloc(size);
}

static void heap_free(size_t size, void *ptr, void *context) {
    (void)size; (void)context;
    free(ptr);
}

static Allocator heap_allocator(void) {
    return (Allocator){ heap_alloc, heap_free, NULL };
}


// -----------------------------------------------------------------------------
// Array_Header
//
// Sits directly before the array data in memory:
//
//   [ Array_Header | item 0 | item 1 | item 2 | ... ]
//   ^                ^
//   h                h + 1  (pointer returned to caller)
//
// _pad brings the struct to 32 bytes so the data that follows is 16-byte aligned.
// -----------------------------------------------------------------------------
typedef struct {
    size_t     length;
    size_t     capacity;
    size_t     _pad;
    Allocator *a;
} Array_Header;

#define ARRAY_INITIAL_CAPACITY 16

#define array(T, a)        ((T *)array_init(sizeof(T), ARRAY_INITIAL_CAPACITY, (a)))
#define array_header(a)    ((Array_Header *)(a) - 1)
#define array_length(a)    (array_header(a)->length)
#define array_capacity(a)  (array_header(a)->capacity)
#define array_pop_back(a)  (array_header(a)->length -= 1)

// Append v to array a. Grows the backing allocation if needed.
// Returns a pointer to the newly inserted element.
#define array_append(a, v) (                                 \
    (a) = array_ensure_capacity((a), 1, sizeof(*(a))),       \
    (a)[array_header(a)->length] = (v),                      \
    &(a)[array_header(a)->length++])

// Swap-remove: replaces element i with the last element. O(1), order not preserved.
#define array_remove(a, i) do {                              \
    Array_Header *_h = array_header(a);                      \
    if ((i) == _h->length - 1) {                             \
        _h->length -= 1;                                     \
    } else if (_h->length > 1) {                             \
        (a)[i] = (a)[_h->length - 1];                       \
        _h->length -= 1;                                     \
    }                                                        \
} while (0)


void *array_init(size_t item_size, size_t capacity, Allocator *a) {
    size_t        size = sizeof(Array_Header) + item_size * capacity;
    Array_Header *h    = a->alloc(size, a->context);  // FIX: original passed 1 arg
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

    size_t new_cap = h->capacity * 2;
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

void array_free(void *a) {
    Array_Header *h    = array_header(a);
    size_t        size = sizeof(*h) + h->capacity * sizeof(*h);
    if (h->a->free) h->a->free(size, h, h->a->context);
}


// =============================================================================
// EXAMPLES
// =============================================================================
int main(void) {
    Allocator al = heap_allocator();

    // -------------------------------------------------------------------------
    printf("=== Append & index ===\n");

    int *nums = array(int, &al);
    for (int i = 1; i <= 10; i++) array_append(nums, i * 10);

    printf("items:    ");
    for (size_t i = 0; i < array_length(nums); i++) printf("%d ", nums[i]);
    printf("\nlength=%zu  capacity=%zu\n\n", array_length(nums), array_capacity(nums));

    // -------------------------------------------------------------------------
    printf("=== Swap-remove ===\n");
    // Removes index 2 (30) by swapping in the last element
    array_remove(nums, 2);
    printf("after remove [2]: ");
    for (size_t i = 0; i < array_length(nums); i++) printf("%d ", nums[i]);
    printf("\n\n");

    // -------------------------------------------------------------------------
    printf("=== Pop back ===\n");
    array_pop_back(nums);
    printf("after pop_back: last item is now %d  length=%zu\n\n",
        nums[array_length(nums) - 1], array_length(nums));

    // -------------------------------------------------------------------------
    printf("=== Growth beyond initial capacity ===\n");

    float *floats = array(float, &al);
    printf("initial capacity: %zu\n", array_capacity(floats));
    for (int i = 0; i < 20; i++) array_append(floats, i * 1.5f);
    printf("after 20 appends: capacity=%zu  length=%zu\n\n",
        array_capacity(floats), array_length(floats));

    // -------------------------------------------------------------------------
    printf("=== Array of structs ===\n");

    typedef struct { int x, y; } Point;
    Point *points = array(Point, &al);
    array_append(points, ((Point){  0,  0 }));
    array_append(points, ((Point){ 10, 20 }));
    array_append(points, ((Point){ -5, 15 }));

    for (size_t i = 0; i < array_length(points); i++) {
        printf("point[%zu]: (%d, %d)\n", i, points[i].x, points[i].y);
    }

    array_free(nums);
    array_free(floats);
    array_free(points);
    return 0;
}
