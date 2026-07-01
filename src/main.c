#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"
#include "vm.h"
#include "test.h"


void usage_err(char *error)
{
    if (error != NULL)
    {
        printf("Error: %s", error);
    }
    printf("Usage: svm [-s <size>] file\n");
    printf("where size is pptional size in words of the workspace, default to %dk\n", DEFAULT_SIZE);
    printf("      file is a valid bytecode file\n");
    exit(USER_ERROR);
}

/* returns size of loaded program */
int load(char *program, Vm * vm) {
    FILE *file = fopen(program, "rb");
    if (file == NULL) {
        usage_err("Bad File");
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    int w_count = file_size / W_SIZE;
    if (w_count > vm->mem_size) {
        printf("Program is too large %d vs %llu", w_count, vm->mem_size);
        return -1;
    }

    int read = fread(vm->memory, W_SIZE, w_count, file);

    if (read != w_count) {
        perror("Failed to read file");
        fclose(file);
        return -1;
    }
    fclose(file);
    return read;
}


int main(int argc, char* argv[]) {
    int mem_size = DEFAULT_SIZE;
    if (argc < 2) {
        usage_err(NULL);
    }
    char* program = argv[1];
    if (strcmp("-s", argv[1]) == 0)
    {
        if (argc < 4) {
            usage_err("wrong number of arguments");
        }
        mem_size = atoi(argv[2]);
        if (mem_size == 0) {
            usage_err("invalid memory size");
        }
        program = argv[3];
    }
    if (strcmp("-t", argv[1]) == 0)
    {
        program = NULL;
    }


    Vm *vm = create_vm(mem_size);

    if (program == NULL)
    {
        return run_tests(vm);
    }


    int size = load(program, vm);

    printf("Welcome to SVM, using %dk memory, program is %s, ends at %d\n", mem_size, program, size);
    return 0;
}
