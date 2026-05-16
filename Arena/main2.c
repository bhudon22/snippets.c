#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// =============================================================================
// ARENA ALLOCATOR — based on bytesbeneath.com/p/the-arena-custom-memory-allocators
//
// Core idea: pre-allocate one big block of memory, then hand out slices of it
// by bumping an offset forward. Freeing is O(1) — just reset the offset to 0.
//
// Benefits over malloc/free:
//   - No fragmentation: allocations are contiguous in memory (cache friendly)
//   - Predictable performance: no hidden bookkeeping per allocation
//   - Simple lifetime model: free everything at once when the arena is done
// =============================================================================


// -----------------------------------------------------------------------------
// Allocator interface
//
// Wrapping the arena behind a generic Allocator lets functions accept *any*
// backing allocator (arena, pool, malloc) without being coupled to one.
// The `context` void pointer carries the allocator's internal state.
// -----------------------------------------------------------------------------
typedef struct {
    void *(*alloc)(size_t size, void *context);
    void  (*free) (size_t size, void *ptr, void *context);
    void  *context;
} Allocator;

// make(T, n, a)  — allocate n items of type T using allocator a
// release(s,p,a) — free a previous allocation (no-op for arenas)
#define make(T, n, a)    ((T *)((a).alloc(sizeof(T) * (n), (a).context)))
#define release(s, p, a) ((a).free((s), (p), (a).context))


// -----------------------------------------------------------------------------
// Arena
//
//   base      — pointer to the start of the backing buffer
//   size      — total capacity in bytes
//   offset    — how many bytes have been handed out (bump pointer)
//   committed — running total of bytes requested (for diagnostics)
// -----------------------------------------------------------------------------
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))  // 16 bytes on 64-bit, safe for all C types

typedef struct {
    void   *base;
    size_t  size;
    size_t  offset;
    size_t  committed;
} Arena;


// -----------------------------------------------------------------------------
// Alignment helpers
//
// CPUs require certain types to live at addresses that are multiples of their
// size (e.g. a uint64_t at an 8-byte boundary). Misaligned access is either
// a bus error (ARM strict mode) or a silent slowdown.
//
// align_forward rounds `ptr` up to the next multiple of `alignment`.
// It uses a bitmask trick that only works when alignment is a power of two:
//   modulo = ptr & (alignment - 1)    ← low bits that need clearing
//   if non-zero, add the gap to reach the next boundary
// -----------------------------------------------------------------------------
#define is_power_of_two(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

static uintptr_t align_forward(uintptr_t ptr, size_t alignment) {
    uintptr_t modulo = ptr & ((uintptr_t)alignment - 1);
    if (modulo) ptr += alignment - modulo;
    return ptr;
}


// -----------------------------------------------------------------------------
// Core allocation
//
// Aligns the current offset, checks there's room, then bumps the offset.
// Returns NULL (0) if the arena is full — no silent overrun.
// -----------------------------------------------------------------------------
static void *arena_alloc_aligned(Arena *a, size_t size, size_t alignment) {
    if (!is_power_of_two(alignment)) return NULL;

    uintptr_t curr = (uintptr_t)a->base + a->offset;
    uintptr_t aligned = align_forward(curr, alignment);
    size_t new_offset = (aligned - (uintptr_t)a->base) + size;

    if (new_offset > a->size) return NULL;  // out of space

    a->committed += size;
    a->offset = new_offset;
    return (void *)aligned;
}

// Public alloc — uses DEFAULT_ALIGNMENT, matches the Allocator interface signature
void *arena_alloc(size_t size, void *context) {
    if (!size) return NULL;
    return arena_alloc_aligned((Arena *)context, size, DEFAULT_ALIGNMENT);
}

// Free is intentionally a no-op — individual items are not freed from an arena.
// The whole arena is freed at once via arena_free_all / freeArena.
void arena_free(size_t size, void *ptr, void *context) {
    (void)size; (void)ptr; (void)context;
}


// -----------------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------------

// arena_init — wrap a caller-provided buffer (stack or heap) in an Arena
Arena arena_init(void *buffer, size_t size) {
    return (Arena){ .base = buffer, .size = size };
}

// arena_free_all — "free" everything by resetting the bump pointer to zero.
// The backing memory is untouched; the arena can be reused immediately.
void arena_free_all(Arena *a) {
    a->offset    = 0;
    a->committed = 0;
}

// arena_alloc_init — convenience macro to build an Allocator from an Arena
#define arena_alloc_init(a) (Allocator){ arena_alloc, arena_free, (a) }


// -----------------------------------------------------------------------------
// Usage example
// -----------------------------------------------------------------------------
int main(void) {
    // One malloc for the whole program's scratch memory
    size_t  capacity = 1024 * 64;           // 64 KB
    void   *buffer   = malloc(capacity);
    Arena   arena    = arena_init(buffer, capacity);
    Allocator al     = arena_alloc_init(&arena);

    // Allocate arrays of different types — all land contiguously in the buffer
    int    *ints   = make(int,    16, al);
    double *floats = make(double,  8, al);
    char   *text   = make(char,   32, al);

    for (int i = 0; i < 16; i++) ints[i]   = i * i;
    for (int i = 0; i <  8; i++) floats[i] = i * 0.5;
    for (int i = 0; i < 31; i++) text[i]   = 'a' + (i % 26);
    text[31] = '\0';

    printf("ints[10]   = %d\n",    ints[10]);
    printf("floats[4]  = %.1f\n",  floats[4]);
    printf("text       = %s\n",    text);
    printf("used       = %zu / %zu bytes\n", arena.offset, arena.size);

    // Free everything in one shot
    free(buffer);
    return 0;
}
