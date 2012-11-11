#include <stdlib.h>
#include "stack.h"

typedef struct Stack{
    void** stack;
    int size, depth;

} Stack;

Stack *newStack(int size) {
    Stack *stack = malloc(sizeof(struct Stack));
    stack->stack = malloc((size_t) (size * sizeof(void*)));
}
void deleteStack(Stack *stack) {
    free(stack->stack);
    free(stack);
}

void *peekIntoStack(Stack *stack) {
    return stack->stack[stack->depth];
}

void *popFromStack(Stack *stack) {
    void * value = stack->stack[stack->depth];
    stack->depth = (stack->depth - 1) % stack->size;
    return value;
}

void pushOnStack(Stack *stack, void *item) {
    stack->depth = (stack->depth + 1) % stack->size;
    stack->stack[stack->depth] = item;
}

void *popIntFromStack(Stack *stack) {
}

void pushIntOnStack(Stack *stack) {
}

void **popObjectArrayFromStack(Stack *stack, int items) {
}
