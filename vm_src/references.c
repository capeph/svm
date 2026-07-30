
#include <_string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "references.h"
#include "hashmap.h"

#define REFERENCE_COUNT 100

// call when a label is encountered,
// record the location of the label and return a collection of previous usages.
ReferenceUsages *add_label(HashMap *refs, char *label, uint64_t value) {
    ReferenceUsages *usage = (ReferenceUsages *)get_map(refs, label);
    if (usage == NULL) {
        usage = malloc(sizeof(ReferenceUsages));
        put_map(refs, label, usage);
        usage->location = value;
        usage->count = 0;
        usage->array_size = REFERENCE_COUNT;
        usage->references = malloc(100 * sizeof(Reference));
    }
    else {
        if (usage->location != UINT64_MAX) {
            printf("Label %s defined multiple times", label);
            exit(-1);
        }
        usage->location = value;
    }
    return usage;
}

void store_ref(ReferenceUsages *usage, uint64_t location, int size) {
    if (usage->count == usage->array_size) {
        int orig_size = usage->array_size * sizeof(Reference);
        usage->array_size += REFERENCE_COUNT;
        Reference *new_references = malloc(orig_size + REFERENCE_COUNT);
        memcpy(new_references, usage->references, orig_size);
        memset((((void *)new_references) + orig_size), 0, REFERENCE_COUNT);
        free(usage->references);
        usage->references = new_references;
    }
    Reference ref = usage->references[usage->count];
    usage->count++;
    ref.location = location;
    ref.size = size;
}

// call when a reference to a label is wncounteres. records, the location
// will return the location of the label if it already defined
ReferenceUsages *add_reference(HashMap *refs, char *label, uint64_t location, int size) {
    ReferenceUsages *usage = (ReferenceUsages *)get_map(refs, label);
    if (usage == NULL) {
        usage = malloc(sizeof(ReferenceUsages));
        put_map(refs, label, usage);
        usage->location = UINT64_MAX;  // mark this as undefined
        usage->count = 0;
        usage->array_size = REFERENCE_COUNT;
        usage->references = malloc(REFERENCE_COUNT * sizeof(Reference));
        memset(usage->references, 0, REFERENCE_COUNT * sizeof(Reference));
    }
    store_ref(usage, location, size);
    return usage;
}
