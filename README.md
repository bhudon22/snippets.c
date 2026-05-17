# snippets.c

A personal collection of C code snippets and toy projects — things worth keeping, studying, and building on.

## Folders

### [Arena](./Arena/)
A custom arena memory allocator. Pre-allocate one large buffer, hand out slices with a bump pointer, free everything in one shot. Includes the `Allocator` interface that the String and DynamicArray modules plug into.

### [DynamicArray](./DynamicArray/)
A generic dynamic array using the stb-style header trick — one allocation holds metadata and array data contiguously. Grows by doubling. Works with any `Allocator` including the Arena.

### [Strings](./Strings/)
A custom `String` type that stores length alongside the pointer, avoiding null-terminator pitfalls. Includes split, join, concat, search, and views. Wires all three modules (Arena + DynamicArray + String) together in one working example.

### [trees](./trees/)
Tree data structures. BST (binary search tree): ordered insert, search, delete, min/max, three traversal orders, and a visual printer. Includes a degenerate case demo showing why self-balancing trees exist.

### [linked_list](./linked_list/)
Singly and doubly linked lists. Covers push/pop at head and tail, insert after, delete, search, reverse, and bidirectional traversal. Key difference: doubly linked gives O(1) tail operations and O(1) removal if you hold a node pointer.

### [hash_table](./hash_table/)
Two collision strategies side by side: separate chaining (linked list per bucket) and open addressing with linear probing (flat array, tombstone deletion). Both use djb2 string hashing. Good comparison of cache locality vs simplicity trade-offs.

### [stacks_queues](./stacks_queues/)
Five classic data structures built on fixed-size arrays: LIFO stack, FIFO queue, deque (double-ended), circular buffer (ring buffer), and priority queue (min-heap). One file per structure, self-contained, no dependencies.

### [dawg](./dawg/)
A Scrabble word finder built on a Trie / DAWG (Directed Acyclic Word Graph) data structure. The arena allocator in this repo was originally developed here.

---

The Arena, DynamicArray, and Strings modules are based on articles by Dylan Falconer at [bytesbeneath.com](https://www.bytesbeneath.com). Bugs found in the original articles are fixed and noted in each folder's README.
