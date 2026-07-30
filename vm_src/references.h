#ifndef REFERENCES_H
#define REFERENCES_H

#include <stdint.h>
#include "hashmap.h"

#define _32BIT 1
#define _64BIT 2
#define _16BIT 3


typedef struct {
    uint64_t location;
    int size;   // 0 unused 1=32bit 2=64bit 3=16bit,
} Reference;

typedef struct {
    uint64_t location;
    Reference *references;   // array;
    int count;
    int array_size;
} ReferenceUsages;

ReferenceUsages *add_label(HashMap *refs, char *label, uint64_t value);
ReferenceUsages *add_reference(HashMap *refs, char *label, uint64_t location, int size);

#endif
