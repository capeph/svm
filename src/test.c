#include "test.h"
#include "interpreter.h"
#include "assembler.h"
#include "hashmap.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void verify(int a, int b, char *string) {
    if (a != b) {
        printf("%s failed, got %d, expected %d\n", string, b, a);
    }
    else {
        printf("%s passed\n", string);
    }
}

void verify_hex(int a, int b, char *string) {
    if (a != b) {
        printf("%s failed, got %x, expected %x\n", string, b, a);
    }
    else {
        printf("%s passed\n", string);
    }
}

void verify_string(char *a, char *b, char *string) {
    if (a == b) {
        printf("%s passed\n", string);
    }
    else if (strcmp(a, b) != 0) {
        printf("%s failed, got %s, expected %s\n", string, b, a);
    }
    else {
        printf("%s passed\n", string);
    }
}

void test_hashmap()
{
    HashMap *map = create_map(4);
    put_map(map, "uno", "first");
    put_map(map, "dos", "second");
    put_map(map, "tres", "third");
    put_map(map, "quatro", "fourth");
    verify_string("first", get_map(map, "uno"), "first");
    verify_string("second", get_map(map, "dos"), "second");
    verify_string("third", get_map(map, "tres"), "third");
    verify_string(NULL, get_map(map, "smurf"), "fourth");
    verify_string("fourth", get_map(map, "quatro"), "fifth");
}


void test_nonspace()
{
    verify(5, nonspace("     word  ", 0), "nonspace - first");
    verify(6, nonspace("     word  ", 6), "nonspace - second");
    verify(-1, nonspace("     word  ", 10), "nonspace - third");
}

void test_space()
{
    verify(-1, space("nospacestring", 0), "space - first");
    verify(9, space("     word  ", 6), "space - second");
}

void test_match()
{
    verify(true, match("LOAD", "    LOAD  ", 4, 8), "match first");
}

void test_assembly(HashMap *is)
{
    verify_hex(0x21408102, assemble(is, " LOAD [R1] , R2"), "LOAD [R1], R2");
    verify_hex(0x21407fd8, assemble(is, " LOAD R127 , [R88]"), "LOAD R127, [R88]");
    verify_hex(0x61400304, assemble(is, " ADD R3, R4"), "ADD R3, R4");
}

int test_load(HashMap *is, Vm *vm)
{
    // place a uint64_t=-5 at location 0
    int64_t *memptr = (int64_t *)(vm->memory);
    *memptr = -5;
    vm->reg[1]=0;
    vm->reg[2]=0;
    vm->reg[3]=7*8;
    interpret(vm, assemble(is, " LOAD R1, [R2]"));
    verify(-5, vm->reg[1], "load to reg");
    interpret(vm, assemble(is, " LOAD [R3], R1"));
    verify(-5, *(memptr+7), "load to memory");
    return 0;
}

int test_add(HashMap *is, Vm *vm)
{
    // place a uint64_t=-5 at location 0
    int64_t *memptr = (int64_t *)(vm->memory);
    *memptr = 12;
    *(memptr + 1) = 34;
    vm->reg[1]=0;
    vm->reg[2]=8;
    vm->reg[3]=0;
    interpret(vm, assemble(is, " LOAD R3, [R1]"));
    verify(12, vm->reg[3], "load to reg");
    interpret(vm, assemble(is, " LOAD R4, [R2]"));
    verify(34, vm->reg[4], "load to reg2");
    interpret(vm, assemble(is, " ADD R3, R4"));
    verify(46, vm->reg[3], "add registers");
    return 0;
}


int test_sub(HashMap *is, Vm *vm)
{
    // place a uint64_t=-5 at location 0
    int64_t *memptr = (int64_t *)(vm->memory);
    *memptr = 12;
    *(memptr + 1) = 34;
    vm->reg[1]=0;
    vm->reg[2]=8;
    vm->reg[3]=0;
    interpret(vm, assemble(is, " LOAD R3, [R1]"));
    verify(12, vm->reg[3], "load to reg");
    interpret(vm, assemble(is, " LOAD R4, [R2]"));
    verify(34, vm->reg[4], "load to reg2");
    interpret(vm, assemble(is, " SUBTRACT R4, R3"));
    verify(22, vm->reg[4], "sub registers");
    return 0;
}



int run_tests(Vm *vm)
{
    HashMap *is = create_instructions();
//    test_hashmap();
//    test_nonspace();
//    test_space();
//    test_match();
//    test_assembly(is);
//    test_load(is, vm);
//    test_add(is, vm);
    test_sub(is, vm);
    destroy_instructions(is);
    return 0;
}
