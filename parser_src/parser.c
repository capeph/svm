#include "lexer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "parser.h"


void advance(ParserContext *ctx) {
    next_token(ctx->lexer);
}

void clear_node_list(Array *array) {
    if (array != NULL) {
        int count = array->last;
        for(int i = 0; i <= count; i++) {
            clear_ast_node(array->data[i]);
        }
        free(array);
    }
}

void clear_multi_op(ast_multi_op *node) {
    clear_ast_node(node->base);
    clear_node_list(node->nodes);
    free(node);
}

void clear_ast_node(void *node) {
    //iterate over subnodes...
    switch (((ast_node *)node)->node_type) {
        case MODULE_NODE :
            clear_multi_op(node);
    default: free(node);
    }
}

Token *last_token(ParserContext *ctx) {
    return ctx->lexer->last_token;
}

bool is_null(ParserContext *ctx) {
    return last_token(ctx) == NULL;
}

bool is_token(ParserContext *ctx, int token_type) {
    return !is_null(ctx) && last_token(ctx)->token_type == token_type;
}



void error(ParserContext *ctx, char *expected) {
    if (is_null(ctx)) {
        printf("Error reading tokens\n");
    }
    else {
        Token *last = last_token(ctx);
        printf("Expecting %s instead of %s(%s)\n", expected, last->name, last->value);
    }
}

ast_binary_op *construct_binop(void *left_node, Token *op, void *right_node, int node_type) {
    ast_binary_op *node = malloc(sizeof(ast_binary_op));
    ((ast_node *)node)->node_type = node_type;
    node->left = left_node;
    node->operator = op;
    node->right = right_node;
    return node;
}

ast_binary_op *make_binop(ParserContext *ctx, void *left_node, node_reader right_reader, int node_type, char *expected) {
//    printf("make_binop\n");
    Token *op = last_token(ctx);
    advance(ctx);
    ast_value_node *right_node = right_reader(ctx);
    if (right_node == NULL) {
        error(ctx, expected);
        return NULL;
    }
    return construct_binop(left_node, op, right_node, node_type);
}

ast_value_node *build_value(char *value, int node_type) {
    ast_value_node *node = malloc(sizeof(ast_value_node));
    ((ast_node *)node)->node_type = node_type;
    node->string_value = value;
    return node;
}



ast_value_node *make_value(ParserContext *ctx, int token_type, int node_type, char *expected) {
//    printf("make_value\n");
    if (is_token(ctx, token_type)) {
        ast_value_node *node = build_value(last_token(ctx)->value, node_type);
        advance(ctx);
        return node;
    }
    error(ctx, expected);
    return NULL;
}



bool is_addop(ParserContext *ctx) {
    return is_token(ctx, PLUS) || is_token(ctx, MINUS);
}

bool is_prefix_op(ParserContext *ctx) {
    return is_token(ctx, PLUS) || is_token(ctx, MINUS) || is_token(ctx, NOT);
}

bool is_mulop(ParserContext *ctx) {
    return is_token(ctx, MULT) || is_token(ctx, DIV);
}

ast_value_node *identifier(ParserContext *ctx) {
    return make_value(ctx, IDENTIFIER, IDENTIFIER_NODE, "identifier");
}

ast_value_node *number(ParserContext *ctx) {
    return make_value(ctx, NUMBER, NUMBER_NODE, "number");
}

ast_value_node *string(ParserContext *ctx) {
    return make_value(ctx, STRING, STRING_NODE, "string");
}

void *get_type(ParserContext *ctx) {
        if (!is_token(ctx, COLON)) {
            error(ctx, "COLON");
            return NULL;
        }
        advance(ctx);
        void *type = identifier(ctx);
//        printf("type %s\n", ((ast_value_node *)type)->string_value);
        return type;
}


void *parameter_def(ParserContext * ctx) {
    // include type defintions
    void *param = identifier(ctx);
    if (is_token(ctx, COLON)) {
        void *type = get_type(ctx);
    }
    return param;
}

bool is_postfix_op(ParserContext *ctx) {
    return is_token(ctx, LPAR);
}

void *base_factor(ParserContext *ctx) {
    if (is_keyword(last_token(ctx))) {
        printf("err %s\n", last_token(ctx)->name);
        return syntactic_expression(ctx);
    }
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
        advance(ctx);
        void *expr = expression(ctx);
        if (is_token(ctx, RPAR)) {
            advance(ctx);
            return expr;
        }
        clear_ast_node(expr);
        error(ctx, "closing paren");
        return NULL;
    }

    error(ctx, "base factor");
    return NULL;
}



Array *expression_list(ParserContext *ctx, node_reader element, int start_token, int end_token)
{
    if (!is_token(ctx, start_token))
    {
        error(ctx, "start token");
        return NULL;
    }
    advance(ctx);
    Array *nodes = create_array(16);
    while (!is_token(ctx, end_token) && !is_token(ctx, EOF_TYPE)) {
        if (is_null(ctx) ){
            error(ctx, "bad expression");
            clear_node_list(nodes);
            return NULL;
        }
        void *exp = element(ctx);
        if (exp == NULL) {
            clear_node_list(nodes);
            return NULL;
        }
        add_to_array(nodes, exp);
        while(is_token(ctx, SEPARATOR)) {
            advance(ctx);
        }
    }
    advance(ctx);
    return nodes;
}

Array *node_list(ParserContext *ctx, node_reader element)
{
    if (!is_token(ctx, LPAR))
    {
        error(ctx, "LPAR");
        return NULL;
    }
    advance(ctx);
    Array *nodes = create_array(16);
    while (!is_token(ctx, RPAR)) {
        if (is_null(ctx) || last_token(ctx)==EOF_TYPE) {
            error(ctx, "complete parameter list");
            clear_node_list(nodes);
            return NULL;
        }
        void *exp = element(ctx);
        if (exp == NULL) {
            clear_node_list(nodes);
            return NULL;
        }
        add_to_array(nodes, exp);
        if (is_token(ctx, COMMA)) {
            advance(ctx);
        }
        while(is_token(ctx, SEPARATOR)) {
            advance(ctx);
        }
    }
    advance(ctx);

    return nodes;
}

ast_node_list *construct_node_list(int node_type, Array *node_list) {
    ast_node_list *multi = malloc(sizeof(ast_node_list));
    multi->nodes = node_list;
    multi->ast.node_type = node_type;
    return multi;
}


ast_multi_op *construct_multi_op(int node_type, void *base, Array *node_list) {
    ast_multi_op *multi = malloc(sizeof(ast_multi_op));
    multi->base = base;
    multi->nodes = node_list;
    multi->ast.node_type = node_type;
    return multi;
}


void *postfix_factor(ParserContext *ctx) {
    void *base = base_factor(ctx);
    while (is_postfix_op(ctx)) {
        if (is_token(ctx, LPAR)) {
            Array *arguments = node_list(ctx, expression);
            if (arguments == NULL) {
                error(ctx, "function arguments");
                return NULL;
            }
            base = construct_multi_op(FUNCTION_CALL, base, arguments);
        }
        else {
            error(ctx, "LPAR");
            return NULL;
        }
    }
    return base;
}


void *prefix_factor(ParserContext *ctx) {
    if (is_prefix_op(ctx)) {
        Token *operator = last_token(ctx);
        advance(ctx);
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


void *factor(ParserContext *ctx) {
    return prefix_factor(ctx);
}

void *term(ParserContext *ctx) {
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


void *algebraic_expression(ParserContext *ctx) {
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


ast_binary_op *definition(ParserContext *ctx) {
//    printf("definition\n");
    if (!(is_token(ctx, LET))) {
        error(ctx, "let");
        return NULL;
    }
    advance(ctx);
    ast_value_node *ident = identifier(ctx);
    if (ident == NULL) {
        error(ctx, "definition name");
        return NULL;
    }
    Array *parameters = NULL;
    if (is_token(ctx, LPAR)) {
        parameters = node_list(ctx, parameter_def);
    }
    void *type = NULL;
    if (is_token(ctx, COLON)) {
        type = get_type(ctx);
    }
    if (is_token(ctx, IS)) {
        Token *token = last_token(ctx);
        advance(ctx);
        void *body = expression(ctx);
        if (parameters != NULL) {
            body = construct_multi_op(FUNCTION_DEF, body, parameters);
//            token = static_token(LAMBDA);
        }
        return construct_binop(ident, token, body, BINARY_NODE);
    }
    error(ctx, "definition value");
    return NULL;
}

void *do_block(ParserContext *ctx) {
    if (!is_token(ctx, DO)) {
        error(ctx, "DO expression");
        return NULL;
    }
    advance(ctx); // consume 'do'
    if (is_token(ctx, INDENT)) {
        Array *nodes = expression_list(ctx, expression, INDENT, DEDENT);
        return construct_node_list(CODE_BLOCK, nodes);
    }
    return expression(ctx);
}

void *if_expression(ParserContext *ctx) {
    return NULL;
}

void *syntactic_expression(ParserContext *ctx) {
    if (is_token(ctx, DO)) {
        return do_block(ctx);
    }
    if (is_token(ctx, LET)) {
        return definition(ctx);
    }
    if (is_token(ctx, IF)) {
        return if_expression(ctx);
    }
    error(ctx, "keyword");
    return NULL;
}


void *expression(ParserContext *ctx) {
    if (is_keyword(last_token(ctx)))
    {
        return syntactic_expression(ctx);
    }
    else {
        return algebraic_expression(ctx);
    }
}




ast_multi_op *module(ParserContext *ctx, char *name) {

    // replace with generic builder
    ast_multi_op *module = malloc(sizeof(ast_multi_op));

    ((ast_node *)module)->node_type = MODULE_NODE;
    module->nodes = create_array(16);
    module->base = build_value(name, IDENTIFIER_NODE);
    while (!is_token(ctx, EOF_TYPE)) {
        if (is_null(ctx)) {
            error(ctx, "definition");
            clear_multi_op(module);
            return NULL;
        }
        if (is_token(ctx, SEPARATOR)) {
            advance(ctx);
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


void print_multi_op(char *prefix, ast_multi_op *module, char *label, bool reverse)
{
    printf("%s + multi(%s)\n", prefix, label);
    char new_prefix[strlen(prefix) + 3];
    snprintf(new_prefix, sizeof(new_prefix), "%s | ", prefix);
    if (!reverse) {
        print_nodes(new_prefix, module->base);
    }
    for(int i = 0; i <= module->nodes->last; i++) {
        print_nodes(new_prefix, get_array(module->nodes, i));
    }
    if (reverse) {
        print_nodes(new_prefix, module->base);
    }
}

void print_node_list(char *prefix, ast_node_list *list, char *label)
{
    printf("%s + block(%s)\n", prefix, label);
    char new_prefix[strlen(prefix) + 3];
    snprintf(new_prefix, sizeof(new_prefix), "%s | ", prefix);
    for(int i = 0; i <= list->nodes->last; i++) {
        print_nodes(new_prefix, get_array(list->nodes, i));
    }
}

void print_nodes(char *prefix, void *root) {
    switch (((ast_node *)root)->node_type) {
    case NUMBER_NODE:
        printf("%s + number(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case STRING_NODE:
        printf("%s + string(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case VARIABLE_NODE:
        printf("%s + variable(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case IDENTIFIER_NODE:
        printf("%s + identifier(%s)\n", prefix, ((ast_value_node *)root)->string_value);
        break;
    case MODULE_NODE:
        print_multi_op(prefix, root, "module", false);
        break;
    case FUNCTION_DEF:
        print_multi_op(prefix, root, "function", true);
        break;
    case FUNCTION_CALL:
        print_multi_op(prefix, root, "call", false);
        break;
    case CODE_BLOCK:
        print_node_list(prefix, root, "do");
        break;
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

    ParserContext *ctx = malloc(sizeof(ParserContext));
    ctx->lexer = get_lexer(fname);

    advance(ctx);
    ast_multi_op *root = module(ctx, name);
    if (root == NULL) {
        printf("Failed to parse\n");
        return NULL;
    }
    print_nodes("", root);

    if (is_null(ctx)) {
        printf("Emtpy module %s\n", name);
    }
    else if (last_token(ctx)->token_type != EOF_TYPE) {
        printf("Unknown token  %s (%s)", last_token(ctx)->name, last_token(ctx)->value);
    }
    return root;
}
