# Handoff — snippets.c

## Project Overview

`snippets.c` is Bernard's personal C learning and reference repo at `/Users/ben/Sources/snippets.c`, mirrored on GitHub at `https://github.com/bhudon22/snippets.c`. The goal is to build a well-documented collection of C data structures and patterns — one concept per folder, one file per structure, each self-contained and compilable standalone. It doubles as a readable reference and a learning archive.

---

## Current Status

The repo is active and growing. Seven folders exist, all committed and pushed to GitHub. The last thing added was a BST (binary search tree) in the `trees/` folder.

---

## Folder Contents

| Folder | Files | What it covers |
|---|---|---|
| `Arena/` | `main.c`, `README.md`, article assets | Arena allocator + Allocator interface |
| `DynamicArray/` | `main.c`, `README.md` | stb-style dynamic array, heap-backed |
| `Strings/` | `main.c`, `README.md`, article .md files | Custom String type wired with Arena + DynamicArray |
| `stacks_queues/` | `stack.c`, `queue.c`, `deque.c`, `circular_buffer.c`, `priority_queue.c`, `README.md` | Five stack/queue variants |
| `linked_list/` | `singly.c`, `doubly.c`, `README.md` | Singly and doubly linked lists |
| `hash_table/` | `chaining.c`, `open_addressing.c`, `README.md` | Two collision strategies |
| `trees/` | `bst.c`, `README.md` | Binary search tree |
| `dawg/` | (original toy project) | Scrabble word finder / DAWG trie |

---

## What Worked

- **One file per structure** — keeps files small and focused. Bernard confirmed this is the right approach.
- **Always add a README per folder** — Bernard asked for this explicitly. Never skip it.
- **Always update the top-level README** when adding a folder — do this as part of the same commit.
- **Always commit and push** after each folder is complete.
- **Compile and run before committing** — all files are verified working with `clang -Wall -Wextra`.
- **Arena/Allocator modules** (from bytesbeneath.com articles) — bugs were fixed; originals had: wrong alloc arg count, str_join array count bug, str_clone missing null byte, Allocator value vs pointer mismatch. All fixed in `Strings/main.c`.
- **File naming** — `.c` extension for all source files (Bernard tried `.c2` once, clarified it was a typo — use `.c`).
- **Compiler**: `clang` on macOS ARM64 (M-series). `xeus-cling` (Jupyter C kernel) is **not compatible** with ARM64 — don't suggest it.

## What Didn't Work / Watch Out For

- `xeus-cling` — not compatible with Apple Silicon. Don't suggest it.
- The bytesbeneath.com articles had several bugs — don't copy them verbatim. See `Strings/main.c` comments marked `// FIX:` for the corrections.
- Don't use `main2.c` naming — Bernard consolidates everything into `main.c` per folder.
- Multiple `main.c` files per folder are fine (Bernard clarified this) — it's just that the naming should make sense.

---

## Next Steps (in order)

1. **AVL tree** — self-balancing BST, add as `trees/avl.c`. The BST README already mentions it as "coming next". The degenerate sorted-insert case in `bst.c` motivates it perfectly.
2. **Red-Black tree** — after AVL, as `trees/red_black.c` (more complex, used in Linux kernel / stdlib maps).
3. **Graph** — adjacency list with BFS and DFS, new `graphs/` folder.
4. **Skip list** — probabilistic sorted structure, interesting alternative to trees.
5. **Trie (standalone)** — a clean extracted version of the DAWG trie logic, separate from the scrabble project.

Bernard's suggested order so far: BST → AVL → (whatever feels right next). He's open to suggestions.

---

## Key Files

| File | Description |
|---|---|
| `README.md` | Top-level index — update this whenever a folder is added |
| `trees/bst.c` | Most recently added file — BST with insert, delete, traversals, visual print |
| `Strings/main.c` | Most complex file — Arena + DynamicArray + String all wired together |
| `Arena/main.c` | Standalone arena allocator reference with alignment and Allocator interface |
| `stacks_queues/priority_queue.c` | Min-heap implementation — most complex of the stacks_queues set |
| `hash_table/open_addressing.c` | Linear probing with tombstone deletion — explains the DELETED state carefully |

---

## Constraints / Gotchas

- **Always add a README** when creating a new folder. Bernard asked for this explicitly — don't skip it.
- **Always update `README.md`** at the repo root when a new folder is added.
- **Always commit and push** at the end. Remote is `origin/main` on GitHub (`bhudon22/snippets.c`).
- **Compile with** `clang -Wall -Wextra` and run before committing — confirm output looks correct.
- **ARM64 Mac** — no compatibility issues with plain C + clang. Avoid Jupyter C kernels.
- **Bernard's style**: small steps, explain the "why" not just the "what", discuss before building anything large. He wants to understand, not just receive finished code.
- **No external dependencies** — every file compiles standalone with `clang file.c`. The Arena/Allocator pattern is copy-pasted into files that need it rather than using headers.
- **Git is not on GitHub remote by default** — `snippets.c` was a local-only repo until it was pushed. Always verify `git push` completes.
