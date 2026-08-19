#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>


#define USER_ERROR -1
#define MEM_ERROR -2
#define FILE_ERROR -3
#define OUT_OF_BOUNDS -4

#define DEFAULT_SIZE 32000
#define W_SIZE sizeof(uint32_t)



typedef struct Array {
    void **data;
    int array_size;
    int last;
} Array;


typedef struct {
    char *key;
    void *data;
    void *next;
} MapNode;

typedef struct {
    MapNode **entries;
    int capacity;
    int size;
} HashMap;


typedef struct TrieNode {
     char value;
     void *data;
     struct TrieNode *nodes;
     int node_count;
} TrieNode;

HashMap *create_map(int size);
void put_map(HashMap *map, char *key, void *data);
void *get_map(HashMap *map, char *key);
void *remove_map(HashMap *map, char *key);
char *get_str(HashMap *cache, char *str, int start, int end);
bool is_digit(char ch, int base);
bool ch_in_list(char ch, char *list);
void trie_add(TrieNode *root, char *key, void *data);
bool trie_has_key(TrieNode *root, char *str);

// return 0 if nothing is found, otherwise lenght of match, match return in *data
void *trie_longest_match(TrieNode *current, char *src, int *pos);
void *trie_match(TrieNode *current, char *key);
void print_trie(TrieNode *root, char *(*printer)(void *));
bool trie_start_char(TrieNode *node, char ch);
Array *create_array(int initial_size);
void add_to_array(Array *array, void *data);
int get_array_size(Array *array);
void *get_array(Array * array, int idx);

#endif
