#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <stdio.h>

uint32_t hash(char *str)
{
  uint32_t hash = 2166136261u;

  for (int i = 0; i < (int)strlen(str); i++) {
    hash ^= (uint8_t)str[i];
    hash *= 16777619;
  }
  return hash;
}

HashMap *create_map(int size) {
    HashMap *map = malloc(sizeof(HashMap));
    map->entries = malloc(size * sizeof(MapNode *));
    map->size = size;
    return map;
}

void put_map(HashMap *map, char *key, void *data)
{
    MapNode *node = malloc(sizeof(MapNode));
    node->data = data;
    node->key = key;
    node->next = NULL;
    uint32_t idx = hash(key) % map->size;
    map->capacity++;
    MapNode* current = map->entries[idx];
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
    MapNode *node = map->entries[idx];
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
    MapNode **nptr = &(map->entries[idx]);
    while (*nptr != NULL && (strcmp(key, (*nptr)->key) != 0)) {
        *nptr = (*nptr)->next;
    }
    if (*nptr == NULL) {
        return NULL;
    }
    void *data = (*nptr)->data;
    MapNode *node = *nptr;
    *nptr = (*nptr)->next;
    free(node);
    map->capacity--;
    return data;
}


char *get_str(HashMap *cache, char *str, int start, int end) {
    char local[256];
//    printf("start=%d, end=%d\n", start, end);
    memcpy(local, str + start, end-start);
    local[end-start] = '\0';
//    printf("got %s\n", local);
    char *cached = get_map(cache, local);
    if (cached != NULL) {
//        printf("found %s\n", cached);
        return cached;
    }
    int len = strlen(local);
    cached = malloc(len+1);   // make room for terminating nil
    memcpy(cached, local, len+1);
    put_map(cache, cached, cached);
//    printf("stored %s\n", cached);
    return cached;
}

bool is_digit(char ch, int base) {
    switch (ch) {
        case '0' :
        case '1' : return true;
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' : return base >= 8;
        case '8' :
        case '9' : return base >= 10;
        case 'a' :
        case 'b' :
        case 'c' :
        case 'd' :
        case 'e' :
        case 'f' : return base == 16;
    }
    return false;
}

bool ch_in_list(char ch, char *list) {
    int len = strlen(list);
    for (int i = 0; i < len; i++) {
        if (list[i] == ch) {
            return true;
        }
    }
    return false;
}

bool trie_start_char(TrieNode *node, char ch) {
    for(int i = 0; i < node->node_count; i++) {
        if (node->nodes[i].value == ch) {
            return true;
        }
    }
    return false;
}

void *trie_longest_match(TrieNode *current, char *line, int *pos) {
    char *src = line + *pos;
//    printf("trie_longest_match '%s' '%c'\n", src, current->value);
    if (src[0] == 0) {
        return NULL;
    }
    for(int i = 0; i < current->node_count; i++) {
        void *match = current->nodes[i].data;
//        printf("checking %c -> %s\n", current->nodes[i].value, (char *)match);
        if (current->nodes[i].value == src[0]) {
//            printf("matched %c to %c  %s\n", current->nodes[i].value, src[0], (char *)match);
            int count = 0;
            void *data = trie_longest_match(&current->nodes[i], src+1, &count);
            if (data != NULL) { // found longer match
//                printf("longer match %s\n", (char *)data);
                *pos += count + 1;
                return data;
            }
            if (match != NULL) {
                *pos += 1;
                return match;
            }
            break;
        }
    }
//    printf("children empty, no match\n");
    return NULL;
}


void *trie_match_internal(TrieNode *current, char *value, int len) {
    if (len == (int)strlen(value)) {
        return current->data;
    }
    for(int i = 0; i < current->node_count; i++) {
        if (current->nodes[i].value == value[len]) {
            return trie_match_internal(&current->nodes[i], value, len + 1);
        }
    }
    return false;
}

void *trie_match(TrieNode *current, char *value) {
    return trie_match_internal(current, value,  0);
}


TrieNode *trie_add_internal(TrieNode *current, char *value,  void *data, int level) {
    int len = strlen(value);
    if (level == len) {
        current->data = data;
//        printf("set data for %c to %s\n", current->value, (char*)current->data);
        return current;
    }
    // check if sub level
    char ch = value[level];
    for(int node_idx = 0; node_idx < current->node_count; node_idx++) {
        if(current->nodes[node_idx].value == ch) {
            return trie_add_internal(&(current->nodes[node_idx]), value, data, level+1);
        }
    }
    if (current->node_count % 5 == 0) {

        int new_size = current-> node_count + 5;
        TrieNode *new_nodes = malloc(new_size * sizeof(TrieNode));
        memset((void *)new_nodes, 0, new_size * sizeof(TrieNode));
        if (current->node_count > 0) {
            memcpy(new_nodes, current->nodes, (current->node_count)*sizeof(TrieNode));
            free((void *)(current->nodes));
        }
        current->nodes = new_nodes;
    }
    TrieNode *insert = &(current->nodes[current->node_count]);
    insert->value = ch;
    insert->data = NULL;
    insert->node_count = 0;
    current->node_count++;
    return trie_add_internal(insert, value, data, level+1);
}


void trie_add(TrieNode *root, char* key, void *data) {
    trie_add_internal(root, key, data, 0);
}

bool trie_has_string(TrieNode *root, char*str) {
    return trie_match_internal(root, str, 0) != NULL;
}



void print_trie(TrieNode *root, char *(*printer)(void *)) {
    char *tokenName = printer(root->data);
    printf("node(%c, %s) ", root->value, tokenName);
    for (int i = 0; i < root->node_count; i++) {
        printf(" - ");
        print_trie(&root->nodes[i], printer);
    }
    if (root->node_count == 0) {
        printf(" .\n");
    }
}


Array *create_array(int initial_size) {
    Array *list = malloc(sizeof(Array));
    list->data = malloc(sizeof(void *) * initial_size);
    list->array_size = initial_size;
    list->last = -1;
    memset(list->data, 0, sizeof(void *) * initial_size);
    return list;
}

void add_to_array(Array *list, void *data) {
    if (list->last == list->array_size -1) {
        void *new_list = malloc(sizeof(void *) * list->array_size * 2);
        memset(list->data, 0, sizeof(void *) * list->array_size * 2);
        memcpy(new_list, list->data, sizeof(void *) * list->array_size);
        free(list->data);
        list->data = new_list;
    }
    list->last++;
    *(list->data + list->last) = data;
}

int get_array_size(Array *list) {
    return list->last;
}

void *get_array(Array *array, int idx) {
    return *(array->data + idx);
}
