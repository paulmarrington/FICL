#include <stdio.h>

typedef struct FICL FICL;
struct FICL {
    char *errors;
    int abort, isCompileMode;
};

FICL *runFICL(FICL *ficl, char *code);