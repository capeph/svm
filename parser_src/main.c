#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "parser.h"



int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Parsing %s\n", argv[1]);
        parse_module(argv[1]);
        exit(0);
    }
    else {
        printf("provide a file, stupid\n");
    }
    return 0;
}
