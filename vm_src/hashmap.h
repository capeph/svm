#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct {
    char *key;
    void *data;
    void *next;
} Node;

typedef struct {
    Node **entries;
    int capacity;
    int size;
} HashMap;


HashMap *create_map(int size);
void put_map(HashMap *map, char *key, void *data);
void *get_map(HashMap *map, char *key);
void *remove_map(HashMap *map, char *key);

#endif
