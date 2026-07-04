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
    uint32_t code[100];
    verify(1, assemble(is, " LOAD [R1] , R2", &code[0]), "singleop");
    assemble(is, " LOAD R127 , [R88]", &code[1]);
    assemble(is, " ADD R3, R4", &code[2]);
    verify(2, assemble(is, " LOAD32 R6, 4711", &code[3]), "const32");
    verify(3, assemble(is, " LOAD64 [R99], -4711", &code[5]), "const64");
    verify(3, assemble(is, " JUMP64 1024", &code[8]), "jump64");

    verify_hex(0x21408102, code[0], "LOAD [R1], R2");
    verify_hex(0x21407fd8, code[1], "LOAD R127, [R88]");
    verify_hex(0x61400304, code[2], "ADD R3, R4");
    verify_hex(0x21800600, code[3], "LOAD R6, ...");
    verify_hex(0x00001267, code[4], "...4711");
    verify_hex(0x21c0e300, code[5], " LOAD [R99]");
    verify_hex(0xFFFFED99, code[6], "...");
    verify_hex(0xFFFFFFFF, code[7], "...-4711");
    verify_hex(0x03c00000, code[8], " JUMP64 1024");
    verify_hex(0x00000400, code[9], "...");
    verify_hex(0x00000000, code[10], "...1024");

}


void set_mem64(Vm *vm, uint64_t offset, int64_t value)
{
    int64_t *memptr = (int64_t *)(vm->memory + offset);
    *memptr = value;
}

int64_t get_mem(Vm *vm, uint64_t offset)
{
    return *((uint64_t *)(vm->memory + offset));
}

void test_load(HashMap *is, Vm *vm)
{
    reset(vm);
    //inject some code to test;
    uint32_t *dest = (uint32_t *)vm->memory;
    int size = 0;
    size += assemble(is, "  LOAD R1, [R2]", dest);
    size += assemble(is, "  LOAD [R3], R1", dest + size);

    set_mem64(vm, 100, -5);
    vm->reg[1]=0;
    vm->reg[2]=100;
    vm->reg[3]=120;
    vm->pc = interpret(vm);
    verify(-5, vm->reg[1], "load to reg");
    vm->pc = interpret(vm);
    verify(-5, get_mem(vm, 120), "load to memory");

}


int test_add(HashMap *is, Vm *vm)
{
    reset(vm);
    //inject some code to test;
    uint32_t *dest = (uint32_t *)vm->memory;
    int size = 0;
    size += assemble(is, " LOAD R3, [R1]", dest);
    size += assemble(is, " LOAD R4, [R2]", dest + size);
    size += assemble(is, " ADD R3, R4", dest + size);

    set_mem64(vm, 80, 12);
    set_mem64(vm, 88, 34);
    vm->reg[1]=80;
    vm->reg[2]=88;
    vm->reg[3]=0;

    vm->pc = interpret(vm);
    verify(12, vm->reg[3], "load to reg");
    vm->pc = interpret(vm);
    verify(34, vm->reg[4], "load to reg2");
    vm->pc = interpret(vm);
    verify(46, vm->reg[3], "add registers");

    return 0;
}


int test_ahead_back(HashMap *is, Vm *vm)
{
    reset(vm);
    //inject some code to test;
    uint32_t *dest = (uint32_t *)vm->memory;
    int size = 0;
    size += assemble(is, " AHEAD 512", dest);
    size += assemble(is, " BACK 1", dest + size);
    size += assemble(is, " JUMP64 98", dest + size);

    vm->pc = interpret(vm);
    verify(512, vm->pc, "ahead");
    vm->pc = 1;
    vm->pc = interpret(vm);
    verify(0, vm->pc, "back");
    vm->pc = 2;
    vm->pc = interpret(vm);
    verify(98, vm->pc, "jump64");

    return 0;
}



int test_sub(HashMap *is, Vm *vm)
{
    reset(vm);
    //inject some code to test;
    uint32_t *dest = (uint32_t *)vm->memory;
    int size = 0;
    size += assemble(is, " LOAD R3, [R1]", dest);
    size += assemble(is, " LOAD R4, [R2]", dest + size);
    size += assemble(is, " SUBTRACT R4, R3", dest + size);

    set_mem64(vm, 80, 12);
    set_mem64(vm, 88, 34);
    vm->reg[1]=80;
    vm->reg[2]=88;
    vm->reg[3]=0;

    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
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
    test_assembly(is);
//    test_load(is, vm);
//    test_add(is, vm);
//    test_sub(is, vm);
    test_ahead_back(is, vm);
    destroy_instructions(is);
    return 0;
}
