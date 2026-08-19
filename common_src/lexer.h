#ifndef LEXER_H
#define LEXER_H
#include "utils.h"
#include <stdio.h>

#define EOF_TYPE 0
#define IDENTIFIER 1
#define INDENT 1
#define DEDENT 2
#define SEPARATOR 2
#define STRING 10
#define NUMBER 11
#define PLUS 101
#define MINUS 102
#define MULT 103
#define DIV 104
#define IS 105
#define NEQ 106
#define NOT 107
#define LPAR 108
#define RPAR 109
#define LT 110
#define GT 111
#define LE 112
#define GE 113
#define EQ 114
#define COMMA 115
#define COLON 116
#define SEMICOLON 117
#define IF 200
#define DO 201


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
    FILE *file;
    char *line;
    int offset;
    int indent;
    HashMap *str_cache;
    HashMap *token_info;
    TrieNode *operators;
    TrieNode *keywords;
    char *white_space;
    Token *last_token;
} LexerContext;

Token *make_token(TokenData *value, char *string, int count);
void return_token(Token *token);
LexerContext *get_lexer(char *source);

Token *next_token(LexerContext *ctx);
void tokenize_file(char *source);

#endif
