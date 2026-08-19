#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Lexing %s\n", argv[1]);
//        tokenize_file(argv[1]);

        LexerContext *ctx = get_lexer(argv[1]);
        Token *token = next_token(ctx);
        while (token != NULL) {
            if (token->token_type == EOF_TYPE) {
                printf("\n EOF reached! \n");
                exit(0);
            }
            printf("got token(%d: %s, %s)\n", token->token_type, token->name, token->value);
            token = next_token(ctx);
        }
        printf("bad token at %d on %s\n", ctx->offset, ctx->line);
        exit(-1);
    }
    else {
        printf("provide a file, stupid\n");
    }
    return 0;
}
