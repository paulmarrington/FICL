/* Copyright (C) 2013 paul@marrington.net, see GPL for license */
#include <stddef.h>
#include <string.h>

typedef char* String;
typedef struct FICL* FICL;
#define ficl(member) ficl->member
typedef struct CompiledWord* CompiledWord;
#define compiledWord(member) compiledWord->member
#define runnable(actor) String (*actor)()

FICL newFICL();
CompiledWord compileFICL(FICL ficl, String source);
String runFICL(FICL ficl, CompiledWord code);
CompiledWord extendFICL(FICL ficl, String name, runnable(actor));
void immediateFICL(FICL ficl);

#define package_local static
#define public
#define func_ref(func) (*func)
#define allocate(structure) (structure) malloc(sizeof(struct structure))
#define toInteger(i) &i
#define toInt(pi) *((int *)pi)
#define toDouble(d) &d
#define null NULL
typedef void* Object;
typedef unsigned int u_int;
typedef int boolean;
#define true 1
#define false 0
void* malloc( size_t size );

/* CompiledWord */
struct CompiledWord {
    String name;
    runnable(actor);
    boolean immediate, variable;
};

/* Dictionary */
typedef void** Dictionary;
#define HASH_SIZE 101

typedef struct _node {
    char *key;
    void *value;
    struct _node *next;
} node;

Dictionary newDictionary() {
    void** dictionary = (void**) malloc(HASH_SIZE * sizeof(node));
    int i;
    for(i = 0; i < HASH_SIZE; i++) dictionary[i] = NULL;
    return dictionary;
}

static unsigned int hash(char *text) {
    unsigned int hashCode=0;
    while (*text) hashCode = *text++ + (hashCode * 31);
    return hashCode % HASH_SIZE;
}

static node* lookup(Dictionary dictionary, String n){
    unsigned int hi = hash(n);
    node* np = dictionary[hi];
    while (np != NULL) {
        if(!strcmp( np->key, n)) return np;
        np = np->next;
    }
    return NULL;
}

void* getFromDictionary(Dictionary dictionary, String key) {
    node* n = lookup(dictionary, key);
    return (n==NULL) ? NULL : n->value;
}

int addToDictionary(Dictionary dictionary, String name, Object value){
    unsigned int hi;
    node* np;
    if((np = lookup(dictionary, name)) == NULL) {
        hi = hash(name);
        /* todo: not freed on error */
        np = (node*) malloc(sizeof(node));
        if(np == NULL) return 0;
        np->key = name;
        if(np->key == NULL) {free(np); return 0; }
        np->next = dictionary[hi];
        dictionary[hi] = np;
    }
    else
        free(np->value);
    
    np->value = value;
    
    return (np->value == NULL) ? 0 : 1;
}

void deleteDictionary(Dictionary dictionary){
    int i;
    for(i = 0; i < HASH_SIZE; i++) {
        if(dictionary[i] != NULL) {
            node *this = dictionary[i];
            while(this != NULL){
                node *next = this->next;
                free(this);
                this = next;
            }
        }
    }
}

/* Stack */
struct Stack {
    void** stack;
    int size, depth;
};
typedef struct Stack* Stack;

Stack newStack(int size) {
    Stack stack = (Stack) malloc(sizeof(struct Stack));
    stack->stack = malloc((size_t) (size * sizeof(Object)));
    stack->size = stack->depth = 0;
}
void deleteStack(Stack stack) {
    free(stack->stack);
    free(stack);
}

Object peekIntoStack(Stack stack) {
    return stack->stack[stack->depth];
}

Object popFromStack(Stack stack) {
    Object value = stack->stack[stack->depth];
    stack->depth = (stack->depth - 1) % stack->size;
    return value;
}

void pushOnStack(Stack stack, Object item) {
    stack->depth = (stack->depth + 1) % stack->size;
    stack->stack[stack->depth] = item;
}

int popIntFromStack(Stack stack) {
    return toInt(popFromStack(stack));
}

void pushIntOnStack(Stack stack, int value) {
    pushOnStack(stack, toInteger(value));
}

void **popObjectArrayFromStack(Stack *stack, int items) {
}

/* Strings */
String substr(str, from) {
    return str + from
}

/* FICL */
struct FICL {
    Dictionary dictionary;
    Stack compiling, secondStack, compilingStack, stack, loopStack;
    int sourcePointer, sourceLength;
    String source;
    boolean debuggingCompile;
};
