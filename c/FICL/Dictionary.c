#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Dictionary.h"

#define HASH_SIZE 101

typedef struct _node{
    char *key;
    void *value;
    struct _node *next;
} node;

Dictionary newDictionary() {
    void** dictionary = malloc(HASH_SIZE * sizeof(node));
    for(int i = 0; i < HASH_SIZE; i++) dictionary[i] = NULL;
    return dictionary;
}

static unsigned int hash(char *text) {
    unsigned int hashCode=0;
    while (*text) hashCode = *text++ + (hashCode * 31);
    return hashCode % HASH_SIZE;
}

static node* lookup(Dictionary dictionary, char *n){
    unsigned int hi = hash(n);
    node* np = dictionary[hi];
    while (np != NULL) {
        if(!strcmp( np->key, n)) return np;
        np = np->next;
    }
    return NULL;
}

char* getFromDictionary(Dictionary dictionary, char* key) {
    node* n = lookup(dictionary, key);
    return (n==NULL) ? NULL : n->value;
}

int addToDictionary(Dictionary dictionary, char* name, void* value){
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
    for(int i = 0; i < HASH_SIZE; i++) {
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
