#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include "hashmap.h"

typedef struct {
    void *dest;
    HashMap *refs;  //str->ReferenceUsages
    HashMap *strcache;  // cache for strings see util.h
    HashMap *is;
} Context;

int nonspace(char *str, int start);
int space(char *str, int start);
bool match(char *match, char *str, int start, int end);

HashMap *create_instructions();
void destroy_instructions(HashMap *map);

uint32_t assemble(Context *context, char *instruction);

int assemble_file(char *filename, char *outfilename);

#endif
