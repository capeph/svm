#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"

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

uint8_t get_byte(Vm * vm, uint8_t reg){
    int64_t regval = vm->reg[reg & 127];
    if(reg & 128) {
        if (regval > vm->mem_size) {
            printf("Memory access out of bounds");
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
        printf("accessing reg %x\n", reg & 127);
        uint64_t regval = vm->reg[reg & 127];
        printf("accessing reg %x which has val %lld\n", reg & 127, regval);

        if (regval > vm->mem_size) {
            printf("Memory access out of bounds");
            exit(OUT_OF_BOUNDS);
        }
        printf("setting 0x[%llx] to %lld\n", regval, value);
        memcpy(vm->memory + regval, &value, sizeof(value));
    }
    else {
        printf("setting reg %x to %lld\n", reg, value);
        vm->reg[reg] = value;
    }
}

void set_byte(Vm *vm, uint8_t reg, uint8_t value) {
    if(reg & 128) {
        printf("accessing reg %x\n", reg & 127);
        uint64_t regval = vm->reg[reg & 127];
        printf("accessing reg %x which has val %lld\n", reg & 127, regval);

        if (regval > vm->mem_size) {
            printf("Memory access out of bounds");
            exit(OUT_OF_BOUNDS);
        }
        printf("setting 0x[%llx] to %d\n", regval, value);
        memcpy(vm->memory + regval, &value, sizeof(value));
    }
    else {
        printf("setting reg %x to %d\n", reg, value);
        vm->reg[reg] = value;
    }
}


void set_flags(Vm *vm, uint64_t value)
{
    if (value == 0) {
        vm->flags = 1 << ZERO_FLAG;
    }
    else {
        if (value >> 63) {
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
    return *(vm->memory + vm->pc);
}

uint64_t get_const(Vm *vm, int size)
{
    uint32_t first = *(vm->memory + vm->pc + 1);
    if (size == 2) {
        uint64_t second = *(vm->memory + vm->pc + 2);
        return (second << 32) & first;
    }
    return first;
}

uint64_t get_pc(Vm *vm) {
    return vm->pc;
}


void set_pc(Vm *vm, uint64_t pc) {
    vm->pc = pc;
}
