#include <stdlib.h>
#include "ficlC.h"
#include "Dictionary.h"

static int RUNNING_WORD_STACK_DEPTH = 32;
static int COMPILING_STACK_DEPTH = 16;
static int OPERATING_STACK_DEPTH = 128;
static int SECOND_STACK_DEPTH = 16;

static FICL *newFICL() {
    FICL *ficl = malloc(sizeof(FICL));
    return ficl;
}

static void compile(FICL *ficl, char *source) {
}

FICL *runFICL(FICL *ficl, char *source) {
    if (ficl == NULL) ficl = newFICL();
    compile(ficl, source);
    return ficl;
}
