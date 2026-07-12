#include "test.h"
#include "interpreter.h"
#include "assembler.h"
#include "hashmap.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void verify(int a, int b, char *string) {
    if (a != b) {
        printf("%s failed, got %d, expected %d\n", string, b, a);
    }
    else {
        printf("%s passed\n", string);
    }
}

void verify_double(double a, double b, char *string) {
    if (fabs(a-b) > a * 1e-6) {
        printf("%s failed, got %f, expected %f\n", string, b, a);
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

void test_basic(Context *context)
{
    uint32_t code[100];
    context->dest = code;
    context->written = 0;
    verify(1, assemble(context, " LOAD [R1] , R2"), "singleop");
    verify_hex(0x21408102, code[0], "LOAD [R1], R2");
    assemble(context, " LOAD R127 , [R88]");
    verify_hex(0x21407fd8, code[1], "LOAD R127, [R88]");
}



void test_assembly(Context *context)
{

    uint32_t code[100];
    context->dest = code;
    verify(1, assemble(context, " LOAD [R1] , R2"), "singleop");
    assemble(context, " LOAD R127 , [R88]");
    assemble(context, " ADD R3, R4");
    verify(2, assemble(context, " LOAD32 R6, 4711"), "const32");
    verify(3, assemble(context, " LOAD64 [R99], -4711"), "const64");
    verify(3, assemble(context, " JUMP64 1024"), "jump64");

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

void test_load(Context *context, Vm *vm)
{
    reset(vm);
    context->written = 0;
    //inject some code to test;
    context->dest = (uint32_t *)vm->memory;
    assemble(context, "  LOAD R1, [R2]");
    assemble(context, "  LOAD [R3], R1");

    set_mem64(vm, 100, -5);
    vm->reg[1]=0;
    vm->reg[2]=100;
    vm->reg[3]=120;
    vm->pc = interpret(vm);
    verify(-5, vm->reg[1], "load to reg");
    vm->pc = interpret(vm);
    verify(-5, get_mem(vm, 120), "load to memory");

}


int test_add(Context *context, Vm *vm)
{
    reset(vm);
    context->written = 0;

    //inject some code to test;
    context->dest = (uint32_t *)vm->memory;
    assemble(context, " LOAD R3, [R1]");
    assemble(context, " LOAD R4, [R2]");
    assemble(context, " ADD R3, R4");

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


int test_ahead_back(Context *context, Vm *vm)
{
    reset(vm);
    context->written = 0;

    //inject some code to test;
    context->dest = vm->memory;
    assemble(context, " AHEAD 512");
    assemble(context, " BACK 4");
    assemble(context, " JUMP64 1024");

    vm->pc = interpret(vm);
    printf("a\n");
    verify(512, vm->pc, "ahead");
    printf("b\n");
    vm->pc = 4;
    printf("c\n");
    vm->pc = interpret(vm);
    printf("d\n");

    verify(0, vm->pc, "back");
    vm->pc = 8;
    vm->pc = interpret(vm);
    verify(1024, vm->pc, "jump64");

    return 0;
}


int test_sub(Context *context, Vm *vm)
{
    reset(vm);
    context->written = 0;

    //inject some code to test;
    context->dest = (uint32_t *)vm->memory;
    assemble(context, " LOAD R3, [R1]");
    assemble(context, " LOAD R4, [R2]");
    assemble(context, " SUBTRACT R4, R3");

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


void test_float(Context *context, Vm *vm) {
    reset(vm);
    context->written = 0;

        //inject some code to test;
    context->dest = (uint32_t *)vm->memory;
    assemble(context, " LOADFLOAT R1, 25.3");
    assemble(context, " ADDFLOAT64 R1, 9.7");
    assemble(context, " FLOATTOINT R0, R1");

    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    verify_double(35.0, get_reg_double(vm, 1), "addfloat");
    verify(35, vm->reg[0], "conversion");
}


void test_cond(Context *context, Vm * vm) {
    reset(vm);
    context->written = 0;

    context->dest = (uint32_t *)vm->memory;
    assemble(context, "LOADBYTECONST R0, 12");
    assemble(context, "LOADBYTECONST R1, 5 IFNP");
    assemble(context, "LOADBYTECONST R2, 5 IFNN");
    assemble(context, "LOAD32 R3, 0");

    assemble(context, "MULTIPLY R3, R0");
    assemble(context, "LOADBYTECONST R4, 7 IFNZ");

    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    vm->pc = interpret(vm);
    vm->pc = interpret(vm);

    verify(12, get_reg(vm, 0), "load");
    verify(0, get_reg(vm, 1), "IFNP");
    verify(5, get_reg(vm, 2), "IFNN");
    verify(0, get_reg(vm, 3), "multiply");
    verify(0, get_reg(vm, 4), "IFNZ");


}

int run_tests(Vm *vm)
{
    Context context;
    context.is = create_instructions();
    context.strcache = create_map(100);
    context.written = 0;

//    test_hashmap();
//    test_nonspace();
//    test_space();
//    test_match();
//    test_basic(&context);

    test_assembly(&context);
    test_load(&context, vm);
    test_add(&context, vm);
    test_sub(&context, vm);
    test_ahead_back(&context, vm);
    test_float(&context, vm);
    test_cond(&context, vm);
    destroy_instructions(context.is);
    return 0;
}
