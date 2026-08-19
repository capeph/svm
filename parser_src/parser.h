#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "utils.h"

#define NUMBER_NODE 1       // ast_value_node
#define STRING_NODE 2       // ast_value_node
#define IDENTIFIER_NODE 3   // ast_value_node
#define FUNCTION_CALL 4   // ast_value_node
#define SIMPLE_EXP 5
#define MODULE_NODE 8
#define DEFINITION_NODE 9
#define BINARY_NODE 10
#define UNARY_NODE 11


typedef struct {
    int node_type;
} typed_ast_node;

typedef struct {
    int node_type;
    char *string_value;
} ast_value_node;

typedef struct {
    int node_type;
    char *identifier;
    Array *nodes;
} ast_multi_op;

typedef struct {
    int node_type;
    Token *operator;
    void *left;
    void *right;
} ast_binary_op;

typedef struct {
    int node_type;
    Token *operator;
    void *value;
} ast_unary_op;

typedef struct {

} AST;


typedef void *(*node_reader)(LexerContext *);

void *factor(LexerContext *ctx);
void *term(LexerContext *ctx);
void *expression(LexerContext *ctx);

AST *parse_module(char *name);

#endif
