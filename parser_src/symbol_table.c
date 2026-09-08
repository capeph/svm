
#include <stdlib.h>
#include "utils.h"

typedef struct type {
    char *name;
} type;

typedef struct symbol {
    char *name;
    type *type;
    Array *arguments;
} symbol;

typedef struct symbols {
    char *scope_name;
    struct symbols *parent;
    HashMap *symbols;
    Array *subscopes;   // Array of symbols
} symbols;

// have a hashmap cach

symbol *get_symbol(char *name, Array *scope, symbols *symbol_table)
{
    return NULL;
}
