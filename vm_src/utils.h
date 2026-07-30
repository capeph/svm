#ifndef utils
#define utils
#include "hashmap.h"

#define USER_ERROR -1
#define MEM_ERROR -2
#define FILE_ERROR -3
#define OUT_OF_BOUNDS -4

#define DEFAULT_SIZE 32000
#define W_SIZE sizeof(uint32_t)

char *get_str(HashMap *cache, char *str, int start, int end);

#endif
