#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "utils.h"

#define NUMBER_NODE 1       // ast_value_node
#define STRING_NODE 2       // ast_value_node
#define IDENTIFIER_NODE 3   // ast_value_node
#define FUNCTION_CALL 4   // ast_value_node
#define FUNCTION_DEF 5
#define SIMPLE_EXP 6
#define MODULE_NODE 8
#define DEFINITION_NODE 9
#define BINARY_NODE 10
#define UNARY_NODE 11
#define VARIABLE_NODE 12
#define INDEX 13


typedef struct {
    int node_type;
} ast_node;

typedef struct {
    ast_node ast;
    char *string_value;
} ast_value_node;

typedef struct {
    ast_node ast;
    void *base;
    Array *nodes;
} ast_multi_op;

typedef struct {
    ast_node ast;
    Token *operator;
    void *left;
    void *right;
} ast_binary_op;

typedef struct {
    ast_node ast;
    Token *operator;
    void *value;
} ast_unary_op;



typedef void *(*node_reader)(LexerContext *);

void clear_ast_node(void *node);

void *factor(LexerContext *ctx);
void *term(LexerContext *ctx);

void *syntactic_expression(LexerContext *ctx) ;
void *algebraic_expression(LexerContext *ctx);
void *expression(LexerContext *ctx);

void *parse_module(char *name);

void print_nodes(char *prefix, void *root);
#endif
