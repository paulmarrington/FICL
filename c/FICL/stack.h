#ifndef _Stack
#define _Stack
typedef struct Stack Stack;
Stack *newStack(int size);
void *peekIntoStack(Stack *stack);
void *popFromStack(Stack *stack);
void pushOnStack(Stack *stack, void *item);

void *popIntFromStack(Stack *stack);
void pushIntOnStack(Stack *stack);

void **popObjectArrayFromStack(Stack *stack, int items);
#endif