#include "lexer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "parser.h"



bool is_token(LexerContext *ctx, int token_type) {
    return ctx->last_token != NULL && ctx->last_token->token_type == token_type;
}

void error(LexerContext *ctx, char *expected) {
    if (ctx->last_token == NULL) {
        printf("Error reading tokens\n");
    }
    else {
        printf("Expecting %s instead of %s\n", expected, ctx->last_token->name);
    }
}

ast_binary_op *make_binop(LexerContext *ctx, void *left_node, node_reader right_reader, int node_type, char *expected) {
//    printf("make_binop\n");
    Token *op = ctx->last_token;
    next_token(ctx);
    ast_value_node *right_node = right_reader(ctx);
    if (right_node == NULL) {
        error(ctx, expected);
        return NULL;
    }
    ast_binary_op *node = malloc(sizeof(ast_binary_op));
    ((ast_node *)node)->node_type = node_type;
    node->left = left_node;
    node->operator = op;
    node->right = right_node;
    return node;
}

ast_value_node *build_value(char *value, int node_type) {
    ast_value_node *node = malloc(sizeof(ast_value_node));
    ((ast_node *)node)->node_type = node_type;
    node->string_value = value;
    return node;
}

ast_value_node *make_value(LexerContext *ctx, int token_type, int node_type, char *expected) {
//    printf("make_value\n");
    if (is_token(ctx, token_type)) {
        ast_value_node *node = build_value(ctx->last_token->value, node_type);
        next_token(ctx);
        return node;
    }
    error(ctx, expected);
    return NULL;
}

bool is_addop(LexerContext *ctx) {
    return is_token(ctx, PLUS) || is_token(ctx, MINUS);
}

bool is_prefix_op(LexerContext *ctx) {
    return is_token(ctx, PLUS) || is_token(ctx, MINUS) || is_token(ctx, NOT);
}

bool is_mulop(LexerContext *ctx) {
    return is_token(ctx, MULT) || is_token(ctx, DIV);
}

ast_value_node *identifier(LexerContext *ctx) {
    return make_value(ctx, IDENTIFIER, IDENTIFIER_NODE, "identifier");
}

ast_value_node *number(LexerContext *ctx) {
    return make_value(ctx, NUMBER, NUMBER_NODE, "number");
}

ast_value_node *string(LexerContext *ctx) {
    return make_value(ctx, STRING, STRING_NODE, "string");
}


bool is_postfix_op(LexerContext *ctx) {
    return is_token(ctx, LPAR);

}

void *base_factor(LexerContext *ctx) {
    if (is_token(ctx, IDENTIFIER)) {
        return identifier(ctx);
    }
    else if (is_token(ctx, NUMBER)) {
        return number(ctx);
    }
    else if (is_token(ctx, STRING)) {
        return string(ctx);
    }
    else if (is_token(ctx, LPAR)) {
        next_token(ctx);
        void *expr = expression(ctx);
        if (is_token(ctx, RPAR)) {
            next_token(ctx);
            return expr;
        }
        error(ctx, "closing paren");
        return NULL;
    }

    error(ctx, "base factor");
    return NULL;
}


void *postfix_factor(LexerContext *ctx) {
    void *base = base_factor(ctx);
    while (is_postfix_op(ctx))
        if (is_token(ctx, LPAR)) {
            next_token(ctx);
            ast_multi_op *call = malloc(sizeof(ast_multi_op));
            ((ast_node *)call)->node_type = FUNCTION_CALL;
            call->nodes = create_array(16);
            call->base = base;
            while (!is_token(ctx, RPAR)) {
                if (ctx->last_token == NULL || ctx->last_token==EOF_TYPE) {
                    error(ctx, "complete parameter list");
                    return NULL;
                }
                void *exp = expression(ctx);
                if (exp == NULL) {
                    return NULL;
                }
                add_to_array(call->nodes, exp);
                if (is_token(ctx, COMMA)) {
                    next_token(ctx);
                }
                while(is_token(ctx, SEPARATOR)) {
                    next_token(ctx);
                }
            }
            next_token(ctx);
            base = call;
        }
    return base;
}


void *prefix_factor(LexerContext *ctx) {
    if (is_prefix_op(ctx)) {
        Token *operator = ctx->last_token;
        next_token(ctx);
        void *operand = prefix_factor(ctx);
        if (operand == NULL) {
            error(ctx, "operand to prefix operator ");
            return NULL;
        }
        ast_unary_op *node = malloc(sizeof(ast_unary_op));
        node->operator = operator;
        ((ast_node *)node)->node_type = UNARY_NODE;
        node->value = operand;
        return node;
    }
    return postfix_factor(ctx);
}


void *factor(LexerContext *ctx) {
    return prefix_factor(ctx);
}

void *term(LexerContext *ctx) {
//    printf("term\n");
    void *left = factor(ctx);
    while (is_mulop(ctx)) {
        ast_binary_op *exp = make_binop(ctx, left, factor, BINARY_NODE, "factor");
        if (exp == NULL) {
            return NULL;
        }
        left = exp;
    }
    return left;
}


void *expression(LexerContext *ctx) {
//    printf("expression\n");
    void *left = term(ctx);
    while (is_addop(ctx)) {
        ast_binary_op *exp = make_binop(ctx, left, term, BINARY_NODE, "term");
        if (exp == NULL) {
            return NULL;
        }
        left = exp;
    }
    return left;
}


ast_binary_op *definition(LexerContext *ctx) {
//    printf("definition\n");
    ast_value_node *ident = identifier(ctx);
    if (ident == NULL) {
        error(ctx, "definition name");
        return NULL;
    }
//    printf("got defintion name %s\n", ident->string_value);
    if (is_token(ctx, IS)) {
        ast_binary_op *def = make_binop(ctx, ident, expression, BINARY_NODE, "expression");
        return def;
    }
    error(ctx, "definition value");
    return NULL;
}



ast_multi_op *module(LexerContext *ctx, char *name) {

    // replace with generic builder
    ast_multi_op *module = malloc(sizeof(ast_multi_op));

    ((ast_node *)module)->node_type = MODULE_NODE;
    module->nodes = create_array(16);
    module->base = build_value(name, IDENTIFIER_NODE);
    while (!is_token(ctx, EOF_TYPE)) {
        if (ctx->last_token == NULL) {
            error(ctx, "definition");
            return NULL;
        }
        if (is_token(ctx, SEPARATOR)) {
            next_token(ctx);
//            printf("got separator, next is %s\n", ctx->last_token->name);
        }
        else {
            void *def = definition(ctx);
            if (def == NULL) {
                return NULL;
            }
            add_to_array(module->nodes, def);
        }
    }
    return module;
}



// todo: add path and file suffix
char *resolve_module_file(char *module_name) {
    return module_name;
}



void print_nodes(char *prefix, void *root) {
    switch (((ast_node *)root)->node_type) {
    case NUMBER_NODE:
        printf("%s number(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case STRING_NODE:
        printf("%s string(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case VARIABLE_NODE:
        printf("%s + variable(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case IDENTIFIER_NODE:
        printf("%s + identifier(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case MODULE_NODE:
        {
            ast_multi_op *module = (ast_multi_op *)root;
            printf("%s + module(%s)\n", prefix, ((ast_value_node *)module->base)->string_value);
            char new_prefix[strlen(prefix) + 3];
            snprintf(new_prefix, sizeof(new_prefix), "%s | ", prefix);
            for(int i = 0; i <= module->nodes->last; i++) {
                print_nodes(new_prefix, get_array(module->nodes, i));
            }
            break;
        }
    case FUNCTION_CALL:
        {
            ast_multi_op *module = (ast_multi_op *)root;
            printf("%s + call(%s)\n", prefix, ((ast_value_node *)module->base)->string_value);
            char new_prefix[strlen(prefix) + 3];
            snprintf(new_prefix, sizeof(new_prefix), "%s | ", prefix);
            print_nodes(new_prefix, module->base);
            for(int i = 0; i <= module->nodes->last; i++) {
                print_nodes(new_prefix, get_array(module->nodes, i));
            }
            break;
        }
    case UNARY_NODE:
        {
            ast_unary_op *unop = (ast_unary_op *)root;
            printf("%s + unary(%s)\n", prefix, unop->operator->name);
            char new_prefix[strlen(prefix) + 3];
            snprintf(new_prefix, sizeof(new_prefix), "%s | ", prefix);
            print_nodes(new_prefix, unop->value);
            break;
        }
    case BINARY_NODE:
        {
            ast_binary_op *binop = (ast_binary_op *)root;
            printf("%s + binop(%s)\n", prefix, binop->operator->name);
            char new_prefix[strlen(prefix) + 3];
            snprintf(new_prefix, sizeof(new_prefix), "%s | ", prefix);
            print_nodes(new_prefix, binop->left);
            print_nodes(new_prefix, binop->right);
            break;
        }
    default:
        break;
    }
}

void *parse_module(char *name)
{
    char *fname = resolve_module_file(name);
    LexerContext *ctx = get_lexer(fname);

    next_token(ctx);
    ast_multi_op *root = module(ctx, name);
    print_nodes("", root);

    if (ctx->last_token == NULL) {
        printf("Emtpy module %s\n", name);
    }
    else if (ctx->last_token->token_type != EOF_TYPE) {
        printf("Unknown token  %s (%s)", ctx->last_token->name, ctx->last_token->value);
    }
    return root;
}
