#ifndef LEXER_H
#define LEXER_H
#include "utils.h"


typedef struct {
    char *name;
    int token_type;
} TokenData;

typedef struct {
    char *name;
    int token_type;
    char *value;
    int count;
} Token;

typedef struct {
    HashMap *str_cache;
    HashMap *token_info;
    TrieNode *operators;
    TrieNode *keywords;
    char *white_space;
} LexerContext;

Token *get_token(TokenData *value, char *string, int count);
void return_token(Token *token);

void tokenize_file(char *source);

#endif
