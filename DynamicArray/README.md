# Dynamic Array

Based on [bytesbeneath.com — Dynamic Arrays in C](https://www.bytesbeneath.com/p/dynamic-arrays-in-c)

## How it works

The header trick (from Sean Barrett / stb libraries): allocate one block containing an `Array_Header` followed immediately by the array data. Return a pointer to the data — the caller gets a plain C array they can index with `[]`. The header is recovered by subtracting 1 from that pointer.

```
[ Array_Header | item 0 | item 1 | item 2 | ... ]
  ^               ^
  h               h + 1  ← pointer returned to caller
```

The `Array_Header` stores length, capacity, and a pointer back to the allocator so the array can grow itself.

```c
typedef struct {
    size_t     length;
    size_t     capacity;
    size_t     _pad;      // pads to 32 bytes for 16-byte aligned data
    Allocator *a;
} Array_Header;
```

Growth strategy: double capacity until it fits. Old block is freed (no-op if arena-backed).

## API

| Macro / Function | Description |
|---|---|
| `array(T, &al)` | Create a new dynamic array of type T |
| `array_append(arr, val)` | Append a value, growing if needed |
| `array_length(arr)` | Number of items |
| `array_capacity(arr)` | Current allocated capacity |
| `array_remove(arr, i)` | Swap-remove at index i — O(1), order not preserved |
| `array_pop_back(arr)` | Remove the last item |
| `array_header(arr)` | Access the raw `Array_Header` |
| `array_free(arr)` | Free the whole array |

## Basic usage

```c
Allocator al = heap_allocator();

int *nums = array(int, &al);
array_append(nums, 10);
array_append(nums, 20);
array_append(nums, 30);

printf("%zu items\n", array_length(nums));  // => 3
printf("%d\n", nums[1]);                    // => 20

array_remove(nums, 0);   // swap-remove: 30 moves into slot 0
printf("%d\n", nums[0]); // => 30

array_free(nums);
```

## Array of structs

```c
typedef struct { int x, y; } Point;

Point *points = array(Point, &al);
array_append(points, ((Point){ 10, 20 }));
array_append(points, ((Point){ -5, 15 }));

for (size_t i = 0; i < array_length(points); i++) {
    printf("(%d, %d)\n", points[i].x, points[i].y);
}
```

## With an arena allocator

Pair with the Arena module (`../Arena/`) for zero-fragmentation allocation.
When arena-backed, `array_remove` is a no-op free — pre-size the arena generously
if the array will grow a lot.

```c
void   *buffer = malloc(1024 * 64);
Arena   arena  = arena_init(buffer, 1024 * 64);
Allocator al   = arena_allocator(&arena);

float *values = array(float, &al);
for (int i = 0; i < 100; i++) array_append(values, i * 0.5f);

free(buffer);  // frees the arena and everything in it, including the array
```

## Bug fixed from the article

`array_init` called `alloc` with one argument, but the `Allocator.alloc` signature takes two (`size` + `context`). Fixed to pass `a->context` as the second argument.
