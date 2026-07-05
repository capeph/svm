#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"
#include "interpreter.h"

Vm *create_vm(uint64_t mem_size)
{
    Vm *vm = malloc(sizeof(Vm));
    if (vm == NULL) {
        perror("Failed to allocate VM");
        exit(MEM_ERROR);
    }

    vm ->memory = malloc(mem_size * W_SIZE);
    if (vm->memory == NULL) {
        perror("Failed to allocate VM memory");
        exit(MEM_ERROR);
    }
    vm->mem_size = mem_size;
    return vm;
}

void destroy_vm(Vm *vm)
{
    free(vm->memory);
    free(vm);
}

int64_t get_reg(Vm *vm, uint8_t reg) {
    int64_t regval = vm->reg[reg & 127];
    if(reg & 128) {
        if (regval > vm->mem_size) {
            printf("Memory access out of bounds");
            exit(OUT_OF_BOUNDS);
        }
        int64_t value = 0;
        memcpy(&value, vm->memory + regval, sizeof(value));
        return value;
    }
    else {
        return regval;
    }
}


double get_reg_double(Vm *vm, uint8_t reg) {
    int64_t regval = vm->reg[reg & 127];
    double value = 0;
    if(reg & 128) {
        if (regval > vm->mem_size) {
            printf("Memory access out of bounds");
            exit(OUT_OF_BOUNDS);
        }
        memcpy(&value, vm->memory + regval, sizeof(value));
    }
    else {
        memcpy(&value, &regval, sizeof(value));
    }
    return value;
}




uint8_t get_byte(Vm * vm, uint8_t reg){
    int64_t regval = vm->reg[reg & 127];
    if(reg & 128) {
        if (regval > vm->mem_size) {
            printf("Memory access out of bounds %lld is more than mem_size %lld\n", regval, vm->mem_size);
            exit(OUT_OF_BOUNDS);
        }
        uint8_t value = 0;
        memcpy(&value, vm->memory + regval, sizeof(value));
        return value;
    }
    else {
        return regval & 0xff;
    }
}

void set_reg(Vm *vm, uint8_t reg, int64_t value) {
    if(reg & 128) {
//        printf("accessing reg %x\n", reg & 127);
        int64_t regval = vm->reg[reg & 127];
//        printf("accessing reg %x which has val %lld\n", reg & 127, regval);

        if (regval > vm->mem_size || regval < 0) {
            printf("Memory access out of bounds");
            exit(OUT_OF_BOUNDS);
        }
//        printf("setting [%lld] to %lld\n", regval, value);
        memcpy(vm->memory + regval, &value, sizeof(value));
    }
    else {
//        printf("setting reg %x to %lld\n", reg, value);
        vm->reg[reg] = value;
    }
}

void set_reg_double(Vm *vm, uint8_t reg, double value) {
    if(reg & 128) {
//        printf("accessing reg %x\n", reg & 127);
        int64_t regval = vm->reg[reg & 127];
//        printf("accessing reg %x which has val %lld\n", reg & 127, regval);

        if (regval > vm->mem_size | regval < 0) {
            printf("Memory access out of bounds");
            exit(OUT_OF_BOUNDS);
        }
//        printf("setting [%lld] to %f\n",si regval, value);
        memcpy(vm->memory + regval, &value, sizeof(value));
    }
    else {
//        printf("setting reg %x to %f\n", reg, value);
        memcpy(&vm->reg[reg], &value, sizeof(value));
    }
}


void set_byte(Vm *vm, uint8_t reg, uint8_t value) {
    if(reg & 128) {
//        printf("accessing reg %x\n", reg & 127);
        uint64_t regval = vm->reg[reg & 127];
//        printf("accessing reg %x which has val %lld\n", reg & 127, regval);

        if (regval > vm->mem_size) {
            printf("Memory access out of bounds");
            exit(OUT_OF_BOUNDS);
        }
//        printf("setting [%lld] to %d\n", regval, value);
        memcpy(vm->memory + regval, &value, sizeof(value));
    }
    else {
//        printf("setting reg %d to %d\n", reg, value);
        vm->reg[reg] = value;
    }
}


void set_flags(Vm *vm, int64_t value)
{
    if (value == 0) {
        vm->flags = 1 << ZERO_FLAG;
    }
    else {
        if (value < 0) {
            vm->flags = 1 << NEGATIVE_FLAG;
        }
        else {
            vm->flags = 1 << POSITIVE_FLAG;
        }
    }
}

void set_flags_double(Vm *vm, double value)
{
    if (value == 0) {
        vm->flags = 1 << ZERO_FLAG;
    }
    else {
        if (value < 0) {
            vm->flags = 1 << NEGATIVE_FLAG;
        }
        else {
            vm->flags = 1 << POSITIVE_FLAG;
        }
    }
}


void set_overflow(Vm *vm) {
    vm->flags = 1 << OVERFLOW_FLAG;
}

void set_double_flags(Vm *vm, double value)
{
    if (value == 0) {
        vm->flags = 1 << ZERO_FLAG;
    }
    else {
        if (value < 0) {
            vm->flags = 1 << NEGATIVE_FLAG;
        }
        else {
            vm->flags = 1 << POSITIVE_FLAG;
        }
    }
}


bool has_flag(Vm *vm, int flag) {
  return vm->flags & (1 << flag);
}


void set_flag(Vm * vm, int flag, bool value) {
    if (value) {
        vm->flags |= (1 << flag);
    }
    else {
        vm -> flags &= ~(1 << flag);
    }
}

uint32_t get_instruction(Vm *vm)
{

    uint32_t *instrp = ((uint32_t *)(vm->memory)) + vm->pc;
    return *instrp;
}

uint64_t get_const(Vm *vm, int size)
{
    uint32_t *firstptr = (uint32_t *)(vm->memory + vm->pc + sizeof(uint32_t));
    uint32_t first = *firstptr;
//    printf("first hex:  %x size=%d\n", first, size);
    if (size == 3) {
        uint32_t *secondptr = (uint32_t *)(vm->memory + vm->pc + 2*sizeof(uint32_t));
        uint64_t second = *secondptr;
//        printf("second hex:  %llx\n", second);
        return (second << 32) | first;
    }
    return first;
}

double get_const_double(Vm *vm)
{
    uint32_t *firstptr = (uint32_t *)(vm->memory + vm->pc + sizeof(uint32_t));
    uint32_t first = *firstptr;
//    printf("first hex:  %x size=%d\n", first, size);
    uint32_t *secondptr = (uint32_t *)(vm->memory + vm->pc + 2*sizeof(uint32_t));
    uint64_t uint64_bits = (((uint64_t)*secondptr) << 32) | first;
//        printf("second hex:  %llx\n", second);
    double result = 0;
    memcpy(&result, &uint64_bits, sizeof(result));
    return result;
}


uint64_t get_pc(Vm *vm) {
    return vm->pc;
}


void set_pc(Vm *vm, uint64_t pc) {
    vm->pc = pc;
}

void reset(Vm *vm) {
    vm->pc = 0;
    vm->run = false;
    memset(vm->memory, vm->mem_size, sizeof(uint8_t));
    memset(vm->reg, 128, sizeof(int64_t));
}

void run(Vm *vm, uint64_t start) {
    vm->pc = start;
    vm->run = true;
    while(vm->run) {
        vm->pc = interpret(vm);
    }
}
