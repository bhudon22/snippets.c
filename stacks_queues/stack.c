#include <stdio.h>
#include <stdbool.h>

// =============================================================================
// STACK — LIFO (Last In, First Out)
//
// Items are pushed and popped from the same end (the top).
// Think of a stack of plates — you add and remove from the top only.
//
// Operations: push, pop, peek — all O(1)
// =============================================================================

#define MAX 8

typedef struct {
    int data[MAX];
    int top;        // index of the next free slot (-1 when empty)
} Stack;

Stack stack_init(void)        { return (Stack){ .top = -1 }; }
bool  stack_is_empty(Stack *s) { return s->top == -1; }
bool  stack_is_full(Stack *s)  { return s->top == MAX - 1; }

bool stack_push(Stack *s, int val) {
    if (stack_is_full(s)) return false;
    s->data[++s->top] = val;
    return true;
}

bool stack_pop(Stack *s, int *out) {
    if (stack_is_empty(s)) return false;
    *out = s->data[s->top--];
    return true;
}

// Peek — read top without removing
bool stack_peek(Stack *s, int *out) {
    if (stack_is_empty(s)) return false;
    *out = s->data[s->top];
    return true;
}

void stack_print(Stack *s) {
    printf("top -> ");
    for (int i = s->top; i >= 0; i--) printf("[%d] ", s->data[i]);
    printf("<- bottom  (size=%d)\n", s->top + 1);
}


int main(void) {
    Stack s = stack_init();

    printf("=== Push ===\n");
    int vals[] = { 10, 20, 30, 40, 50 };
    for (int i = 0; i < 5; i++) {
        stack_push(&s, vals[i]);
        stack_print(&s);
    }

    printf("\n=== Peek ===\n");
    int top;
    stack_peek(&s, &top);
    printf("top = %d\n", top);

    printf("\n=== Pop ===\n");
    int val;
    while (stack_pop(&s, &val)) {
        printf("popped %d  |  ", val);
        stack_print(&s);
    }

    printf("\n=== Overflow guard ===\n");
    for (int i = 0; i < MAX + 2; i++) {
        bool ok = stack_push(&s, i * 10);
        if (!ok) printf("push %d failed — stack full\n", i * 10);
    }

    return 0;
}
