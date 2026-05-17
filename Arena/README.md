# Arena Allocator

Based on [bytesbeneath.com — The Arena: Custom Memory Allocators](https://www.bytesbeneath.com/p/the-arena-custom-memory-allocators)

## What it is

An arena allocator pre-allocates one large block of memory and hands out slices of it by bumping an offset forward. Freeing is O(1) — just reset the offset to zero. No per-object bookkeeping, no fragmentation.

**Use it when** a group of objects share the same lifetime and you want to free them all at once (e.g. a frame, a request, a parse tree).

## The structs

```c
typedef struct {
    void *base;       // start of the backing buffer
    size_t size;      // total capacity in bytes
    size_t offset;    // bytes handed out so far (the bump pointer)
    size_t committed; // running total requested (for diagnostics)
} Arena;
```

The `Allocator` interface decouples your code from the arena — swap in malloc or a pool allocator without changing call sites:

```c
typedef struct {
    void *(*alloc)(size_t size, void *context);
    void  (*free) (size_t size, void *ptr, void *context);
    void  *context;
} Allocator;
```

## API

| Function / Macro | Description |
|---|---|
| `arena_init(buffer, size)` | Wrap a buffer (heap or stack) in an Arena |
| `arena_alloc_init(&arena)` | Build an Allocator backed by the arena |
| `make(T, n, allocator)` | Allocate n items of type T |
| `release(size, ptr, allocator)` | No-op for arenas; exists for interface symmetry |
| `arena_free_all(&arena)` | Reset offset to 0 — reuse the arena immediately |

## Basic usage

```c
void *buffer = malloc(1024 * 64);          // one malloc for everything
Arena arena  = arena_init(buffer, 1024 * 64);
Allocator al = arena_alloc_init(&arena);

int  *ids    = make(int,  100, al);        // 100 ints, contiguous
char *names  = make(char, 256, al);        // right after the ints

// ... use ids and names ...

arena_free_all(&arena);                    // reset — ids/names are invalid now
free(buffer);                              // one free at the end
```

## Stack-backed arena (no heap at all)

```c
uint8_t buf[4096];
Arena arena = arena_init(buf, sizeof(buf));
Allocator al = arena_alloc_init(&arena);

float *data = make(float, 64, al);
```

## Reusing an arena across phases

```c
Arena arena = arena_init(malloc(MB), MB);
Allocator al = arena_alloc_init(&arena);

for (int frame = 0; frame < 100; frame++) {
    Particle *particles = make(Particle, 512, al);
    simulate(particles, 512);
    arena_free_all(&arena);   // wipe and reuse each frame
}
```

## Alignment

All allocations are aligned to `DEFAULT_ALIGNMENT` (16 bytes on 64-bit), which is safe for every standard C type. The `align_forward` helper uses a power-of-two bitmask trick:

```c
modulo = ptr & (alignment - 1);   // low bits that overshoot the boundary
if (modulo) ptr += alignment - modulo;  // round up
```

Misaligned access on ARM64 is either a bus error or a silent slowdown — this handles it correctly.
