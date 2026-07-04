#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdbool.h>

/* Flags */

#define ZERO_FLAG  1
#define POSITIVE_FLAG 2
#define NEGATIVE_FLAG 4
#define OVERFLOW_FLAG 8;


typedef struct {
    uint64_t pc;
    int64_t reg[128];
    uint32_t flags;
    uint8_t *memory;
    uint64_t mem_size;
} Vm;

Vm *create_vm(uint64_t mem_size);
void destroy_vm(Vm *vm);

int64_t get_reg(Vm *vm, uint8_t reg);
void set_reg(Vm *vm, uint8_t reg, int64_t value);

uint8_t get_byte(Vm *vm, uint8_t reg);
void set_byte(Vm *vm, uint8_t reg, uint8_t value);

void set_flags(Vm *vm, uint64_t value);
void set_double_flags(Vm *vm, double value);
void set_overflow(Vm *vm);

bool has_flag(Vm *vm, int flag);
void set_flag(Vm * vm, int flag, bool value);

uint64_t get_pc(Vm *vm);
void set_pc(Vm *vm, uint64_t pc);

uint32_t get_instruction(Vm *vm);
uint64_t get_const(Vm *vm, int size);
void reset(Vm* vm);


#endif
