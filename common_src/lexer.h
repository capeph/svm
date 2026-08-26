#ifndef LEXER_H
#define LEXER_H
#include "utils.h"
#include <stdio.h>


#define TOKEN(token_type, number) ((token_type << 8)| (number))

// token types
#define LEXER  0
#define OPERATOR  1
#define KEYWORD  2
#define ATOM  4

//tokens
#define EOF_TYPE TOKEN(LEXER, 0)
#define INDENT TOKEN(LEXER, 1)
#define DEDENT TOKEN(LEXER, 2)
#define SEPARATOR TOKEN(LEXER, 3)

#define IDENTIFIER TOKEN(ATOM, 0)
#define STRING TOKEN(ATOM, 1)
#define NUMBER TOKEN(ATOM, 2)
#define CHARACTER TOKEN(ATOM, 3)

#define PLUS TOKEN(OPERATOR, 1)
#define MINUS TOKEN(OPERATOR, 2)
#define MULT TOKEN(OPERATOR, 3)
#define DIV TOKEN(OPERATOR, 4)
#define IS TOKEN(OPERATOR, 5)
#define NEQ TOKEN(OPERATOR, 6)
#define NOT TOKEN(OPERATOR, 7)
#define LT TOKEN(OPERATOR, 8)
#define GT TOKEN(OPERATOR, 9)
#define LE TOKEN(OPERATOR, 10)
#define GE TOKEN(OPERATOR, 11)
#define EQ TOKEN(OPERATOR, 12)
#define LAMBDA TOKEN(OPERATOR, 13)

#define COMMA TOKEN(OPERATOR, 13)
#define COLON TOKEN(OPERATOR, 14)
#define SEMICOLON TOKEN(OPERATOR, 15)

#define LPAR TOKEN(OPERATOR, 16)
#define RPAR TOKEN(OPERATOR, 17)
#define LBRACKET TOKEN(OPERATOR, 18)
#define RBRACKET TOKEN(OPERATOR, 19)
#define LBRACE TOKEN(OPERATOR, 20)
#define RBRACE TOKEN(OPERATOR, 21)

#define LET TOKEN(KEYWORD, 0)
#define DO TOKEN(KEYWORD, 1)
#define IF TOKEN(KEYWORD, 2)


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

Token *static_token(int token);
Token *next_token(LexerContext *ctx);
void tokenize_file(char *source);
bool is_operator(Token *token);
bool is_keyword(Token *token);

#endif
