#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "hashmap.h"
#include <stdio.h>

uint32_t hash(char *str)
{
  uint32_t hash = 2166136261u;

  for (int i = 0; i < strlen(str); i++) {
    hash ^= (uint8_t)str[i];
    hash *= 16777619;
  }
  return hash;
}

HashMap *create_map(int size) {
    HashMap *map = malloc(sizeof(HashMap));
    map->entries = malloc(size * sizeof(Node*));
    map->size = size;
    return map;
}

void put_map(HashMap *map, char *key, void *data)
{
    Node *node = malloc(sizeof(Node));
    node->data = data;
    node->key = key;
    node->next = NULL;
    uint32_t idx = hash(key) % map->size;
    map->capacity++;
    Node* current = map->entries[idx];
    if (current == NULL) {
        map->entries[idx] = node;
    }
    else {
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = node;
    }
}

void *get_map(HashMap *map, char *key) {
    uint32_t idx = hash(key) % map->size;
    Node *node = map->entries[idx];
    while (node != NULL && (strcmp(key, node->key) != 0)) {
        node = node->next;
    }
    if( node == NULL) {
        return NULL;
    }
    else {
        return node->data;
    }
}

void *remove_map(HashMap *map, char *key) {
    uint32_t idx = hash(key) % map->size;
    Node **nptr = &(map->entries[idx]);
    while (*nptr != NULL && (strcmp(key, (*nptr)->key) != 0)) {
        *nptr = (*nptr)->next;
    }
    if (*nptr == NULL) {
        return NULL;
    }
    void *data = (*nptr)->data;
    Node *node = *nptr;
    *nptr = (*nptr)->next;
    free(node);
    map->capacity--;
    return data;
}
