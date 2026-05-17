# Trees

Tree data structures. Starting with the BST — the foundation everything else builds on.

---

## [bst.c](./bst.c) — Binary Search Tree

Every node satisfies: **left subtree values < node < right subtree values**. This ordering makes search, insert, and delete O(log n) on a balanced tree.

```
        80
    70
        60
50
            45
        40
            35
    30
        20
            10
```
*(read: right branch on top, left on bottom)*

```c
BST t = bst_init();
bst_insert(&t, 50);
bst_insert(&t, 30);
bst_insert(&t, 70);

bst_contains(&t, 30);   // => true
bst_min(&t);            // => 30
bst_max(&t);            // => 70

bst_delete(&t, 30);     // handles leaf, one child, two children
bst_inorder(&t);        // => 50 70  (always sorted)
bst_free(&t);
```

### API

| Function | Description | Complexity |
|---|---|---|
| `bst_insert(&t, val)` | Insert value (duplicates ignored) | O(log n) avg |
| `bst_contains(&t, val)` | Search | O(log n) avg |
| `bst_delete(&t, val)` | Remove value | O(log n) avg |
| `bst_min(&t)` | Smallest value | O(log n) avg |
| `bst_max(&t)` | Largest value | O(log n) avg |
| `bst_height(&t)` | Longest root-to-leaf path | O(n) |
| `bst_inorder(&t)` | Print sorted ascending | O(n) |
| `bst_preorder(&t)` | Print root before children | O(n) |
| `bst_postorder(&t)` | Print root after children | O(n) |
| `bst_print(&t)` | Visual sideways tree | O(n) |
| `bst_free(&t)` | Free all nodes | O(n) |

### Traversal order

| Order | Visit sequence | Use for |
|---|---|---|
| In-order | left → node → right | Sorted output |
| Pre-order | node → left → right | Copying / serialising |
| Post-order | left → right → node | Freeing (children before parent) |

### Delete — three cases

1. **Leaf** — just remove it
2. **One child** — replace the node with its child
3. **Two children** — find the in-order successor (smallest value in right subtree), copy its value into the node, delete the successor below

### The degenerate case

Inserting already-sorted data produces a straight line — every node has only a right child. Height becomes n-1 instead of log n, and all operations degrade to O(n):

```
1
  2
    3
      4
        5
```

This is the motivation for self-balancing trees (AVL, Red-Black) which restructure themselves to keep height at O(log n) regardless of insertion order.

---

*Coming next: AVL tree (self-balancing BST)*
