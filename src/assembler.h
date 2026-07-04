#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include "hashmap.h"


int nonspace(char *str, int start);
int space(char *str, int start);
bool match(char *match, char *str, int start, int end);

HashMap *create_instructions();
void destroy_instructions(HashMap *map);

uint32_t assemble(HashMap *builders, char *instruction, uint32_t *dest);


#endif
