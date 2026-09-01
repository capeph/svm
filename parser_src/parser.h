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
#define CODE_BLOCK 14


typedef struct {
    int node_type;
} ast_node;

typedef struct {
    ast_node ast;
    char *string_value;
} ast_value_node;

typedef struct {
    ast_node ast;
    Array *nodes;
} ast_node_list;

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


typedef struct symbol_table {
    struct symbol_table *parent;
    void *ast_node;
    HashMap *symbols;
} symbol_table;

typedef struct {
    LexerContext *lexer;
} ParserContext;

typedef void *(*node_reader)(ParserContext *);

void clear_ast_node(void *node);

void *factor(ParserContext *ctx);
void *term(ParserContext *ctx);

void *syntactic_expression(ParserContext *ctx) ;
void *algebraic_expression(ParserContext *ctx);
void *expression(ParserContext *ctx);

void *parse_module(char *name);

void print_nodes(char *prefix, void *root);
#endif
