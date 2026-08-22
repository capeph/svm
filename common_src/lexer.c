
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "utils.h"
#include "lexer.h"

#define LINE_BUFFER_SIZE 512
#define TAB_SPACES 4
TokenData eof_data = {"EOF", EOF_TYPE};
TokenData number_data = {"NUMBER", NUMBER};
TokenData string_data = {"STRING", STRING};
TokenData identifier_data = {"IDENTIFIER", IDENTIFIER};
TokenData indent_data = {"INDENT", INDENT};
TokenData dedent_data = {"DEDENT", DEDENT};
TokenData separator_data = {"SEPARATOR", SEPARATOR};


bool is_decimal_separator(char ch) {
    return ch == '.';
}

bool is_exponent_separator(char ch, int base) {
    // use the exponent separator 'p' for both octal and hex, since it denotes a left shift...
    return (base == 10 && ch == 'e') || (base == 16 && ch == 'p') || (base == 8 && ch == 'p');
}

bool is_sign(char ch) {
    return ch == '-' || ch == '+';
}

bool is_space(char *str, int offset) {
    return *(str + offset) == ' ' || *(str + offset) == '\t';
}

int get_indent(char *line, int *offset) {
    int indent = 0;
    int len = strlen(line);
    for(int i = 0; i < len; i++) {
        char ch = *(line + i);
        if (ch == ' ') {
            indent++;
        }
        else if (ch == '\t') {
            indent += TAB_SPACES;
        }
        else {
            break;
        }
        (*offset)++;
    }
    return indent;
}

int get_base_prefix(char *line, int *offset) {
    char first = *(line + *offset);
    char second = *(line + *offset + 1);
    if (first == '0') {
        if (second == 'x' || second == 'X') {
            return 16;
        }
        else if (second == 'o' || second == 'O') {
            return 8;
        }
        else if (second == 'b' || second == 'B') {
            return 2;
        }
    }
    return 10;
}

Token *get_number(LexerContext *ctx, char *line, int *offset)
{
    int pos = *offset;
    int base = get_base_prefix(line, offset);
    if (base != 10) {  //adjust read position for the prefix (always two characters)
        pos += 2;
    }
    int int_start = pos;
    while (is_digit(line[pos], base)) {
        pos++;
    }
    if (is_decimal_separator(line[pos])) {
        pos++;
        while (is_digit(line[pos], base)) {
            pos++;
        }
    }
    if (pos == int_start) {
        return NULL;  // no number found
    }
    if (is_exponent_separator(line[pos], base)) {
        pos++;
        if (is_sign(line[pos])) {
            pos++;
        }
        while (is_digit(line[pos], base)) {
            pos++;
        }
    }

    char *num = get_str(ctx->str_cache, line, *offset, pos);
    *offset = pos;
    return make_token(&number_data, num, 1);
}

Token *get_operator(LexerContext *ctx, char *line, int *offset) {
    int matched = 0;
    TokenData *match = trie_longest_match(ctx->operators, line + *offset, &matched);
    if (matched > 0)
    {
//        printf("got longest match %s(%d)\n", match, matched);
        char *token_string = get_str(ctx->str_cache, line, *offset, *offset + matched);
        Token *token = make_token(match, token_string, 1);
        *offset += matched;
        return token;
    }
    return NULL;  // unknown operator!
}

Token *get_string(LexerContext *ctx, char *line, int *offset) {
    int pos = *offset;
    if (line[pos] != '"') {
        return NULL;  // not called at start of string
    }
    pos++;
    int str_start = pos;
    while(line[pos] != '"' && line[pos] != '\0') {
        if (line[pos] == '\\') {
            pos++; // skip next char! escaped
            if (line[pos] =='\0') {
                printf("escaped end of line!");
                return NULL;
            }
        }
        pos++;
//        printf("start = %d, pos = %d\n", str_start, pos);
    }
    if (line[pos] == '"') {
        char *str = get_str(ctx->str_cache, line, str_start, pos);
//        printf("got string(%lu) %s\n", strlen(str), str);
        *offset = pos+1;
        return make_token(&string_data, str, 1);
    }
    printf("malformed string!");
    return NULL;
}

char *get_character(LexerContext *ctx, char *line, int *offset)
{
    return NULL;
}


Token *get_identifier(LexerContext *ctx, char *line, int *offset) {
    int pos = *offset;
    int ident_start = pos;
    while(!(line[pos] == '\0' ||
        trie_start_char(ctx->operators, line[pos]) ||
            ch_in_list(line[pos], ctx->white_space))) {
//        printf("skipping char %c\n", line[pos]);
        pos++;
    }
//    printf("ident ends with '%c'\n", line[pos]);
    if (pos == ident_start) {
        printf("no ident found\n");
        return NULL; // no number found
    }
    char *id = get_str(ctx->str_cache, line, ident_start, pos);
    TokenData *keyword = trie_match(ctx->keywords, id);
    *offset = pos;
    if (keyword != NULL) {
        return make_token(keyword, id, 1);
    }
    return make_token(&identifier_data, id, 1);

}

bool is_linebreak(char ch) {
    return ch == '\n';
}


void *get_white_space(LexerContext *ctx, char* line, int *pos) {
    if (ch_in_list(line[*pos], ctx->white_space)) {
//        printf("got white space\n");
        char ch = line[*pos];
        *pos += 1;
        switch (ch) {
            case ' ' : return "SPACE";
            case '\t' : return "TAB" ;
            case '\r' : return "CR" ;
            case '\n' : return "LF" ;
            default : return "NOISE";
        }
    }
    return NULL;
}

void skip_white_space(LexerContext *ctx, char* line, int *pos) {
    int i = 0;
    while(get_white_space(ctx, line, pos) != NULL){
        i++;
    }
//    printf("(%d spaces)\n", i);
}


Token *read_token(LexerContext *ctx, char *line, int *pos)
{
    skip_white_space(ctx, line, pos);
    char *str = line + *pos;
    int read = 0;
    Token *token = NULL;
    if (read == 0) {
        token = get_operator(ctx, str, &read);
    }
    if (read == 0) {
        token = get_number(ctx, str, &read);
    }
    if (read == 0) {
        token = get_string(ctx, str, &read);
    }
    if (read == 0) {
        token = get_identifier(ctx, str, &read);
    }
    if (read == 0) {
        printf ("unknown symbol at %d on %s\n", *pos, line);
        return NULL;
    }
    else {
        *pos += read;
        return token;
    }
}

void tokenize_line(LexerContext *ctx, char *line) {
    int pos = 0;
    int end = (int)strlen(line);
    int indent = get_indent(line, &pos);
    printf("Indent(%d)\n", indent);
    while(pos < end) {
        Token *tok = read_token(ctx, line, &pos);

        if (tok == NULL) {
            printf("bad token at %d on %s\n", pos, line);
            exit(-1);
        }
        else {
            printf("got token(%d: %s, %s)\n", tok->token_type, tok->name, tok->value);
        }
    }
    printf("end\n");
}


TokenData *describe_token(char *name, int type)
{
    TokenData *data = malloc(sizeof(TokenData));
    data->name = name;
    data->token_type = type;
    return data;
}

char *token_data_printer(void *data) {
    if (data == NULL) {
        return "-";
    }
    else {
        TokenData *td = data;
        return td->name;
    }
}


Token *make_token(TokenData *type, char *string, int count) {
    Token *token = malloc(sizeof(Token));
    token ->token_type = type->token_type;
    token ->value = string;
    token ->name = type->name;
    token->count = count;
    return token;
}

void return_token(Token *token) {
    free(token);
}


void setup_operators(LexerContext *ctx)
{
    TrieNode *ops = malloc(sizeof(TrieNode));
    ops->value = 0;
    ops->node_count = 0;
    ops->data = NULL;
    trie_add(ops, "+", describe_token("PLUS", PLUS));
    trie_add(ops, "-", describe_token("MINUS", MINUS));
    trie_add(ops, "*", describe_token("MULT", MULT));
    trie_add(ops, "/", describe_token("DIV", DIV));
    trie_add(ops, "=", describe_token("IS", IS));
    trie_add(ops, "!=", describe_token("NEQ", NEQ));
    trie_add(ops, "!", describe_token("NOT", NOT));
    trie_add(ops, "(", describe_token("LPAR", LPAR));
    trie_add(ops, ")", describe_token("RPAR", RPAR));
    trie_add(ops, "[", describe_token("LBRACKET", LBRACKET));
    trie_add(ops, "]", describe_token("RBRACKET", RBRACKET));
    trie_add(ops, "{", describe_token("LBRACE", LBRACE));
    trie_add(ops, "}", describe_token("RBRACE", RBRACE));
    trie_add(ops, "<", describe_token("LT", LT));
    trie_add(ops, ">", describe_token("GT", GT));
    trie_add(ops, "<=", describe_token("LE", LE));
    trie_add(ops, ">=", describe_token("GE", GE));
    trie_add(ops, "==", describe_token("EQ", EQ));
    trie_add(ops, ",", describe_token("COMMA", COMMA));
    trie_add(ops, ":", describe_token("COLON", COLON));
    trie_add(ops, ";", describe_token("SEMICOLON", SEMICOLON));
    ctx->operators = ops;
}

void setup_keywords(LexerContext *ctx) {
    TrieNode *keywords = malloc(sizeof(TrieNode));
    keywords->value = 0;
    keywords->node_count = 0;
    keywords->data = NULL;
    trie_add(keywords, "if", describe_token("IF", IF));
    trie_add(keywords, "do", describe_token("DO", DO));
    ctx->keywords = keywords;
}


LexerContext *get_lexer(char *source) {
    LexerContext *ctx = malloc(sizeof(LexerContext));
    ctx->file = fopen(source, "r");
    if (ctx->file == NULL) {
        printf("provide a valid file\n");
        return NULL;
    }
    setup_operators(ctx);
    setup_keywords(ctx);
    ctx->str_cache = create_map(1024);
    ctx->line = malloc(LINE_BUFFER_SIZE);
    ctx->offset = 0;
    ctx->indent = 0;
    memset(ctx->line, 0, LINE_BUFFER_SIZE);
    ctx->white_space = " \t\n\r";

    return ctx;
}


Token *next_token(LexerContext *ctx) {
    int indent = ctx->indent;
    bool newline = false;
    bool retry;
    do {
        retry = false;
        newline = false;
        while (ctx->line[ctx->offset] == '\0') {
            if (fgets(ctx->line, LINE_BUFFER_SIZE, ctx->file) == NULL) {
                ctx->last_token = make_token(&eof_data, "", 1);
                return ctx->last_token;
            }
            int last = strlen(ctx->line) - 1;
            if (ctx->line[last] == '\n') {
                ctx->line[last] = '\0';
            }
            ctx->offset = 0;
            indent = get_indent(ctx->line, &ctx->offset);
            newline = true;
        }
        if (indent > ctx->indent) {
            ctx->indent = indent;
            ctx->last_token = make_token(&indent_data, get_str(ctx->str_cache, ctx->line, 0, ctx->offset), indent);
        }
        else if (indent < ctx->indent) {
            ctx->indent = indent;
            ctx->last_token =  make_token(&dedent_data, get_str(ctx->str_cache, ctx->line, 0, ctx->offset), indent);
        }
        else if (newline && indent == ctx->indent) {
            if (ctx->last_token == NULL || ctx->last_token->token_type == separator_data.token_type) {
                retry = true;
            }
                ctx->last_token = make_token(&separator_data, "", 0);
        }
        else {
            ctx->last_token = read_token(ctx, ctx->line, &ctx->offset);
        }
    } while (retry);

    return ctx->last_token;
}
