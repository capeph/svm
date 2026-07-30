#include "hashmap.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


char *get_str(HashMap *cache, char *str, int start, int end) {
    char local[256];
    memcpy(local, str + start, end-start);
    local[end=start] = '\0';
    char *cached = get_map(cache, local);
    if (cached != NULL) {
        return cached;
    }
    cached = malloc(strlen(local));
    put_map(cache, cached, cached);
    return cached;
}
