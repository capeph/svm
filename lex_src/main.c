#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Lexing %s\n", argv[1]);
        tokenize_file(argv[1]);
    }
    else {
        printf("provide a file, stupid\n");
    }
    return 0;
}
