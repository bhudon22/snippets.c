# Strings, Dynamic Arrays & Arena — Wired Together

Based on three articles by Dylan Falconer at [bytesbeneath.com](https://www.bytesbeneath.com):

- [The Arena: Custom Memory Allocators](https://www.bytesbeneath.com/p/the-arena-custom-memory-allocators)
- [Dynamic Arrays in C](https://www.bytesbeneath.com/p/dynamic-arrays-in-c)
- [Custom Strings in C](https://www.bytesbeneath.com/p/custom-strings-in-c)

`main.c` wires all three together into a working example with bugs from the articles fixed.

---

## The three modules

### 1. Allocator interface

Shared glue between all three modules. A function-pointer pair + opaque context so every function can accept any backing allocator without being coupled to one.

```c
typedef struct {
    void *(*alloc)(size_t size, void *context);
    void  (*free) (size_t size, void *ptr, void *context);
    void  *context;
} Allocator;
```

### 2. Arena

One `malloc` up front, bump-pointer allocation from it, one `free` at the end. See `../Arena/README.md` for full details.

```c
void   *buffer = malloc(1024 * 256);
Arena   arena  = arena_init(buffer, 1024 * 256);
Allocator al   = arena_allocator(&arena);
// ... all allocations come from al ...
free(buffer);
```

### 3. Dynamic Array

Header trick (Sean Barrett / stb-style): one allocation holds an `Array_Header` followed immediately by the array data. The caller gets back a plain C pointer they can index with `[]`.

```c
typedef struct {
    size_t     length;
    size_t     capacity;
    size_t     _pad;      // pads to 32 bytes for alignment
    Allocator *a;
} Array_Header;
```

| Macro | Description |
|---|---|
| `array(T, &al)` | Create a new dynamic array of type T |
| `array_append(arr, val)` | Append a value, growing if needed |
| `array_length(arr)` | Number of items |
| `array_capacity(arr)` | Current allocated capacity |
| `array_remove(arr, i)` | Swap-remove at index i (O(1), order not preserved) |
| `array_pop_back(arr)` | Remove last item |
| `array_header(arr)` | Access the raw header (length, capacity) |

```c
int *nums = array(int, &al);
array_append(nums, 42);
array_append(nums, 99);
printf("%zu items\n", array_length(nums));  // => 2
array_remove(nums, 0);                      // 99 swaps into slot 0
```

### 4. Custom String

Stores length alongside the pointer. Keeps the null terminator so `.data` passes directly to `printf` and any C stdlib function.

```c
typedef struct {
    size_t  len;
    char   *data;
} String;
```

Two flavours:
- **Owning** — allocated via the arena, have their own memory
- **View** — `{len, ptr}` into existing memory, no allocation, no copy

| Function / Macro | Description |
|---|---|
| `Str("literal")` | Wrap a string literal (no allocation) |
| `str_init(len, &al)` | Allocate an empty string of given length |
| `str_clone(s, &al)` | Allocate a copy |
| `str_concat(s1, s2, &al)` | Allocate s1 + s2 |
| `str_substring(s, start, end, &al)` | Allocate a copy of s[start..end) |
| `str_view(s, start, end)` | View into s[start..end) — no allocation |
| `str_substring_view(haystack, needle)` | View from first match to end — no allocation |
| `str_equal(a, b)` | True if same length and content |
| `str_contains(haystack, needle)` | True if needle found |
| `str_index_of(haystack, needle)` | Index of first match, or `(size_t)-1` |
| `str_split(s, delim, &al)` | Split into a dynamic `String` array |
| `str_join(arr, joiner, &al)` | Join a `String` array with a separator |

```c
String s = Str("the quick brown fox");

// Search
printf("%zu\n", str_index_of(s, Str("fox")));   // => 16
printf("%s\n",  str_contains(s, Str("cat")) ? "yes" : "no"); // => no

// View — no allocation, not null-terminated at the tail
String view = str_substring_view(s, Str("brown"));
printf("%.*s\n", (int)view.len, view.data);      // => brown fox

// Split and rejoin
String *words   = str_split(s, Str(" "), &al);
String rejoined = str_join(words, Str("|"), &al);
printf("%s\n", rejoined.data);                   // => the|quick|brown|fox
```

---

## Bugs fixed from the articles

| Location | Bug | Fix |
|---|---|---|
| `array_init` | `alloc` called with 1 arg, but signature takes 2 | Added missing `a->context` argument |
| `str_join` | Used `s->len` (first string's length) as the array count | Changed to `h->length` (array header count) |
| `str_clone` | Allocated `len` bytes, missing null terminator | `str_init` handles `len + 1` correctly |
| All string/array functions | `Allocator` passed by value in arena article, by pointer in string/array articles | Unified to `Allocator *` throughout |
