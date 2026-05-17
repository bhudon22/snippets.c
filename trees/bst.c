#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// =============================================================================
// BINARY SEARCH TREE (BST)
//
// Every node satisfies: left subtree values < node < right subtree values.
// This ordering makes search, insert, and delete all O(log n) on a balanced
// tree — degrading to O(n) if the tree becomes a straight line (e.g. inserting
// already-sorted data).
//
// Operations:
//   insert, search, delete — O(log n) average, O(n) worst
//   min, max, height       — O(log n) average
//   in-order traversal     — O(n), visits nodes in sorted order
// =============================================================================

typedef struct Node {
    int          val;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node  *root;
    size_t length;
} BST;

BST bst_init(void) { return (BST){0}; }

static Node *node_create(int val) {
    Node *n  = malloc(sizeof(Node));
    n->val   = val;
    n->left  = NULL;
    n->right = NULL;
    return n;
}


// -----------------------------------------------------------------------------
// Insert — O(log n) average
// -----------------------------------------------------------------------------
static Node *insert(Node *node, int val) {
    if (!node) return node_create(val);
    if      (val < node->val) node->left  = insert(node->left,  val);
    else if (val > node->val) node->right = insert(node->right, val);
    // equal: duplicates ignored
    return node;
}

void bst_insert(BST *t, int val) {
    t->root = insert(t->root, val);
    t->length++;
}


// -----------------------------------------------------------------------------
// Search — O(log n) average
// -----------------------------------------------------------------------------
static Node *search(Node *node, int val) {
    if (!node || node->val == val) return node;
    return val < node->val ? search(node->left, val) : search(node->right, val);
}

bool bst_contains(BST *t, int val) { return search(t->root, val) != NULL; }


// -----------------------------------------------------------------------------
// Min / Max
// -----------------------------------------------------------------------------
static Node *min_node(Node *node) {
    while (node->left) node = node->left;
    return node;
}

static Node *max_node(Node *node) {
    while (node->right) node = node->right;
    return node;
}

int bst_min(BST *t) { return min_node(t->root)->val; }
int bst_max(BST *t) { return max_node(t->root)->val; }


// -----------------------------------------------------------------------------
// Delete — O(log n) average
//
// Three cases:
//   1. No children    — just remove the node
//   2. One child      — replace the node with its child
//   3. Two children   — find the in-order successor (smallest in right subtree),
//                       copy its value here, then delete the successor below
// -----------------------------------------------------------------------------
static Node *delete(Node *node, int val) {
    if (!node) return NULL;

    if (val < node->val) {
        node->left = delete(node->left, val);
    } else if (val > node->val) {
        node->right = delete(node->right, val);
    } else {
        // Found — handle the three cases
        if (!node->left && !node->right) {      // case 1: leaf
            free(node);
            return NULL;
        }
        if (!node->left) {                      // case 2a: only right child
            Node *tmp = node->right;
            free(node);
            return tmp;
        }
        if (!node->right) {                     // case 2b: only left child
            Node *tmp = node->left;
            free(node);
            return tmp;
        }
        // Case 3: two children — replace with in-order successor
        Node *successor = min_node(node->right);
        node->val       = successor->val;
        node->right     = delete(node->right, successor->val);
    }
    return node;
}

void bst_delete(BST *t, int val) {
    if (!bst_contains(t, val)) return;
    t->root = delete(t->root, val);
    t->length--;
}


// -----------------------------------------------------------------------------
// Height — longest path from root to a leaf
// -----------------------------------------------------------------------------
static int height(Node *node) {
    if (!node) return -1;
    int lh = height(node->left);
    int rh = height(node->right);
    return 1 + (lh > rh ? lh : rh);
}

int bst_height(BST *t) { return height(t->root); }


// -----------------------------------------------------------------------------
// Traversals — each visits every node exactly once: O(n)
//
//   In-order   (left, root, right) — visits in sorted ascending order
//   Pre-order  (root, left, right) — useful for copying/serialising the tree
//   Post-order (left, right, root) — useful for freeing (children before parent)
// -----------------------------------------------------------------------------
static void inorder(Node *node) {
    if (!node) return;
    inorder(node->left);
    printf("%d ", node->val);
    inorder(node->right);
}

static void preorder(Node *node) {
    if (!node) return;
    printf("%d ", node->val);
    preorder(node->left);
    preorder(node->right);
}

static void postorder(Node *node) {
    if (!node) return;
    postorder(node->left);
    postorder(node->right);
    printf("%d ", node->val);
}

void bst_inorder(BST *t)   { inorder(t->root);   printf("\n"); }
void bst_preorder(BST *t)  { preorder(t->root);  printf("\n"); }
void bst_postorder(BST *t) { postorder(t->root); printf("\n"); }


// -----------------------------------------------------------------------------
// Print — sideways tree, right subtree on top, left on bottom
//
//         right
//   root
//         left
// -----------------------------------------------------------------------------
static void print_tree(Node *node, int indent) {
    if (!node) return;
    print_tree(node->right, indent + 4);
    printf("%*s%d\n", indent, "", node->val);
    print_tree(node->left,  indent + 4);
}

void bst_print(BST *t) { print_tree(t->root, 0); }


// -----------------------------------------------------------------------------
// Free
// -----------------------------------------------------------------------------
static void free_tree(Node *node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

void bst_free(BST *t) { free_tree(t->root); t->root = NULL; t->length = 0; }


// =============================================================================
// EXAMPLES
// =============================================================================
int main(void) {
    BST t = bst_init();

    printf("=== Insert ===\n");
    int vals[] = { 50, 30, 70, 20, 40, 60, 80, 10, 35, 45 };
    for (int i = 0; i < 10; i++) bst_insert(&t, vals[i]);
    printf("length=%zu  height=%d\n\n", t.length, bst_height(&t));

    printf("=== Tree (read: right branch on top, left on bottom) ===\n");
    bst_print(&t);

    printf("\n=== Traversals ===\n");
    printf("in-order   (sorted): "); bst_inorder(&t);
    printf("pre-order  (root first): "); bst_preorder(&t);
    printf("post-order (root last):  "); bst_postorder(&t);

    printf("\n=== Min / Max ===\n");
    printf("min=%d  max=%d\n", bst_min(&t), bst_max(&t));

    printf("\n=== Search ===\n");
    printf("contains 40: %s\n", bst_contains(&t, 40) ? "yes" : "no");
    printf("contains 55: %s\n", bst_contains(&t, 55) ? "yes" : "no");

    printf("\n=== Delete ===\n");
    printf("delete 20 (leaf):\n");
    bst_delete(&t, 20);
    bst_print(&t);

    printf("\ndelete 30 (two children):\n");
    bst_delete(&t, 30);
    bst_print(&t);

    printf("\ndelete 70 (two children):\n");
    bst_delete(&t, 70);
    bst_print(&t);

    printf("in-order after deletes: "); bst_inorder(&t);

    printf("\n=== Degenerate case — sorted insert becomes a linked list ===\n");
    BST bad = bst_init();
    for (int i = 1; i <= 7; i++) bst_insert(&bad, i);
    printf("inserted 1..7 in order: height=%d (optimal would be 2)\n", bst_height(&bad));
    bst_print(&bad);
    bst_free(&bad);

    bst_free(&t);
    return 0;
}
