
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "utils.h"
#include "lexer.h"

#define TAB_SPACES 4
TokenData number_data = {"NUMBER", 1};
TokenData string_data = {"STRING", 2};
TokenData identifier_data = {"IDENTIFIER", 3};


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
    if (is_exponent_separator(line[pos], base)) {
        pos++;
        if (is_sign(line[pos])) {
            pos++;
        }
        while (is_digit(line[pos], base)) {
            pos++;
        }
    }
    if (pos == int_start) {
        return NULL; // no number found
    }

    char *num = get_str(ctx->str_cache, line, *offset, pos);
    *offset = pos;
    return get_token(&number_data, num, 1);
}

Token *get_operator(LexerContext *ctx, char *line, int *offset) {
    int matched = 0;
    TokenData *match = trie_longest_match(ctx->operators, line + *offset, &matched);
    if (matched > 0)
    {
//        printf("got longest match %s(%d)\n", match, matched);
        char *token_string = get_str(ctx->str_cache, line, *offset, *offset + matched);
        Token *token = get_token(match, token_string, 1);
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
        return get_token(&string_data, str, 1);
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
        return get_token(keyword, id, 1);
    }
    return get_token(&identifier_data, id, 1);

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
//    token = get_white_space(ctx, str, &read);
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
        printf("got token(%s)\n", token->name);
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

void tokenize_file(char *source) {
    FILE *file = fopen(source, "r");
    if (file == NULL) {
        printf("provide a valid file\n");
        exit(-1);
    }
    LexerContext ctx;
    ctx.token_info = create_map(500);

    TrieNode ops;
    ops.value = 0;
    ops.node_count = 0;
    ops.data = NULL;
    trie_add(&ops, "+", describe_token("PLUS", 101));
    trie_add(&ops, "-", describe_token("MINUS", 102));
    trie_add(&ops, "*", describe_token("MULT", 103));
    trie_add(&ops, "/", describe_token("DIV", 104));
    trie_add(&ops, "=", describe_token("IS", 105));
    trie_add(&ops, "!=", describe_token("NEQ", 106));
    trie_add(&ops, "!", describe_token("NOT", 107));
    trie_add(&ops, "(", describe_token("LPAR", 108));
    trie_add(&ops, ")", describe_token("RPAR", 109));
    trie_add(&ops, "<", describe_token("LT", 110));
    trie_add(&ops, ">", describe_token("GT", 111));
    trie_add(&ops, "<=", describe_token("LE", 112));
    trie_add(&ops, ">=", describe_token("GE", 113));
    trie_add(&ops, "==", describe_token("EQ", 114));
    trie_add(&ops, ",", describe_token("COMMA", 115));
    trie_add(&ops, ":", describe_token("COLON", 116));
    trie_add(&ops, ";", describe_token("SEMICOLON", 117));
    ctx.operators = &ops;

    print_trie(&ops);

    TrieNode keywords;
    keywords.value = 0;
    keywords.node_count = 0;
    keywords.data = NULL;
    trie_add(&keywords, "if", describe_token("IF", 200));
    trie_add(&keywords, "do", describe_token("DO", 201));
    ctx.keywords = &keywords;

    ctx.white_space = " \t\n\r";
    ctx.str_cache = create_map(1024);
    char line[256];
    memset(line, 0, sizeof(line));
    while (fgets(line, sizeof(line), file) != NULL) {
        int len = strlen(line);

        if (line[len - 1] == '\n') {
            line[len -1] = '\0';
        }
        printf("tokenizing: '%s'\n",line);
        tokenize_line(&ctx, line);
    }

}


Token *get_token(TokenData *type, char *string, int count) {
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
