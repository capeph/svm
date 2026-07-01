#include <_string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "vm.h"
#include "opcodes.h"
#include <stdckdint.h>

#define OPCODE(instruction) ((instruction >> 22) & 0x3ff)
#define FLAGS(instruction) ((instruction >> 16) & 0x3f)
#define TARGET(instruction) ((instruction >> 8) & 0xff)
#define DATA(instruction) (instruction & 0xff)
#define OFFSET(instruction) ((instruction) & 0xffff)

/* modifiers */

#define SIZE(instruction) ((instruction >> 22 ) && 3) // size is low bits of the opcode


uint64_t interpret(Vm *vm, uint32_t instruction) {
    int size = SIZE(instruction);
    uint64_t new_pc = vm->pc + size;
    if (FLAGS(instruction) & vm->flags)
    {   // conditional skip
        return new_pc;
    }
    int opcode = OPCODE(instruction);
    printf("Opcode is %d (%x)\n", opcode, opcode);

    switch (OPCODE(instruction)) {
    case HALT :
        printf("System halted\n");
        exit(0);
     case AHEAD :
        new_pc = vm->pc + OFFSET(instruction);
        break;
    case BACK :
    {
        uint64_t offset = OFFSET(instruction);
        if (offset > vm->pc) {
            printf("negative address");
            exit(-1);
        }
        new_pc = vm->pc - offset;
        break;
    }
    case JUMP :
        new_pc = get_reg(vm, TARGET(instruction));
        break;
    case JUMPHALF :
    case JUMPCONST :
        new_pc = get_const(vm, size);
        break;
    case JUMPZERO :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (vm->flags & ZERO_FLAG) {
            new_pc = get_reg(vm, TARGET(instruction));
        }
        break;
    case JUMPZEROHALF :
    case JUMPZEROCONST :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (vm->flags & ZERO_FLAG) {
            new_pc = get_const(vm, size);
        }
        break;
    case JUMPNOTZERO :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (!(vm->flags & ZERO_FLAG)) {
            new_pc = get_reg(vm, TARGET(instruction));
        }
        break;
    case JUMPNOTZEROHALF :
    case JUMPNOTZEROCONST :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (!(vm->flags & ZERO_FLAG)) {
            new_pc = get_const(vm, size);
        }
        break;
    case JUMPPOSITIVE :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (vm->flags & POSITIVE_FLAG) {
            new_pc = get_reg(vm, TARGET(instruction));
        }
        break;
    case JUMPPOSITIVEHALF :
    case JUMPPOSITIVECONST :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (vm->flags & POSITIVE_FLAG) {
            new_pc = get_const(vm, size);
        }
        break;
    case JUMPNOTPOSITIVE :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (!(vm->flags & POSITIVE_FLAG)) {
            new_pc = get_reg(vm, TARGET(instruction));
        }
        break;
    case JUMPNOTPOSITIVEHALF :
    case JUMPNOTPOSITIVECONST :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (!(vm->flags & POSITIVE_FLAG)) {
            new_pc = get_const(vm, size);
        }
        break;
    case JUMPNEGATIVE :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (vm->flags & NEGATIVE_FLAG) {
            new_pc = get_reg(vm, TARGET(instruction));
        }
        break;
    case JUMPNEGATIVEHALF :
    case JUMPNEGATIVECONST :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (vm->flags & NEGATIVE_FLAG) {
            new_pc = get_const(vm, size);
        }
        break;
    case JUMPNOTNEGATIVE :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (!(vm->flags & NEGATIVE_FLAG)) {
            new_pc = get_reg(vm, TARGET(instruction));
        }
        break;
    case JUMPNOTNEGATIVEHALF :
    case JUMPNOTNEGATIVECONST :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (!(vm->flags & NEGATIVE_FLAG)) {
            new_pc = get_const(vm, size);
        }
        break;
    case CALL :
        set_reg(vm, DATA(instruction), new_pc);
        set_flags(vm, 0);
        new_pc = get_reg(vm, TARGET(instruction));
        break;
    case CALLHALF :
    case CALLCONST :
        set_reg(vm, DATA(instruction), new_pc);
        set_flags(vm, 0);
        new_pc = get_reg(vm, get_const(vm, size));
        break;
    case LOADBYTE :
    {
        uint8_t byte = get_byte(vm, DATA(instruction));
        set_byte(vm, TARGET(instruction), byte);
        break;
    }
    case LOADBYTECONST :
    {
        uint8_t byte = DATA(instruction);
        set_byte(vm, TARGET(instruction), byte);
        break;
    }
    case LOAD :
    {
        printf("load %x %x\n", TARGET(instruction), DATA(instruction));
        int64_t value = get_reg(vm, DATA(instruction));
        printf("value = %lld\n", value);
        set_reg(vm, TARGET(instruction), value);
        set_flags(vm, value);
        break;
    }
    case LOADHALF :
    case LOADCONST :
    {
        uint64_t value = get_const(vm, size);
        set_reg(vm, TARGET(instruction), value);
        set_flags(vm, value);
        break;
    }
    case INTTOFLOAT :
    {
        double value = (double)get_reg(vm, DATA(instruction));
        int64_t result = 0;
        memcpy(&result, &value, sizeof(result));
        set_reg(vm, TARGET(instruction), result);
        set_double_flags(vm, result);
        break;
    }
    case FLOATTOINT :
    {
        int64_t value = get_reg(vm, DATA(instruction));
        double double_val;
        memcpy(&double_val, &value, sizeof(double_val));
        value = (uint64_t)double_val;
        set_reg(vm, TARGET(instruction), value);
        set_flags(vm, value);
        break;
    }
    case SHIFTLEFT :
    {
        uint64_t shift = get_reg(vm, DATA(instruction));
        uint64_t value = get_reg(vm, TARGET(instruction)) << shift;
        set_reg(vm, TARGET(instruction), value);
        set_flags(vm, value);
        break;
    }
    case SHIFTRIGHT :
    {
        uint64_t shift = get_reg(vm, DATA(instruction));
        uint64_t value = get_reg(vm, TARGET(instruction)) >> shift;
        set_reg(vm, TARGET(instruction), value);
        set_flags(vm, value);
        break;
    }
    case ROTLEFT :
    {
        uint64_t shift = get_reg(vm, DATA(instruction)) % sizeof(uint64_t);
        uint64_t value = get_reg(vm, TARGET(instruction));
        uint32_t result = value << shift | value >> (sizeof(uint64_t) - shift);
        set_reg(vm, TARGET(instruction), result);
        set_flags(vm, result);
        break;
    }
    case ROTRIGHT :
    {
        uint64_t shift = get_reg(vm, DATA(instruction)) % sizeof(uint64_t);
        uint64_t value = get_reg(vm, TARGET(instruction));
        uint32_t result = value >> shift | value << (sizeof(uint64_t) - shift);
        set_reg(vm, TARGET(instruction), result);
        set_flags(vm, result);
        break;
    }
    case COMPARE :
    {
        uint64_t first = get_reg(vm, TARGET(instruction));
        uint64_t second = get_reg(vm, DATA(instruction));
        int64_t diff = first - second;
        set_flags(vm, diff);
        break;
    }
    case ADD :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = 0;
        if (ckd_add(&dest, target, data)) {
            set_overflow(vm);
        }
        else {
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case ADDHALF :
    case ADDCONST :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = 0;
        if (ckd_add(&dest, target, data)) {
            set_overflow(vm);
        }
        else {
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case SUBTRACT :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = 0;
        if (ckd_sub(&dest, target, data)) {
            set_overflow(vm);
        }
        else {
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case SUBTRACTHALF :
    case SUBTRACTOCONST :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = 0;
        if (ckd_sub(&dest, target, data)) {
            set_overflow(vm);
        }
        else {
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case MULTIPLY :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = 0;
        if (ckd_mul(&dest, target, data)) {
            set_overflow(vm);
        }
        else {
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case MULTIPLYHALF :
    case MULTIPLYCONST :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = 0;
        if (ckd_mul(&dest, target, data)) {
            set_overflow(vm);
        }
        else {
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case DIVIDE :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = 0;
        if (data == 0) {
            set_overflow(vm);
        }
        else {
            dest = target / data;
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case DIVIDEHALF :
        break;
    case DIVIDECONST :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_const(vm, size);
        int64_t dest = 0;
        if (data == 0) {
            set_overflow(vm);
        }
        else {
            dest = target / data;
            set_reg(vm, target_reg, dest);
            set_flags(vm, target);
        }
        break;
    }
    case ADDFLOAT :
        break;
    case ADDFLOATCONST :
        break;
    case SUBTRACTFLOAT :
        break;
    case SUBTRACTFLOATCONST :
        break;
    case MULTIPLYFLOAT :
        break;
    case MULTIPLYFLOATCONST :
        break;
    case DIVIDEFLOAT :
        break;
    case DIVIDEFLOATCONST :
        break;

    case NOT :
        break;
    case AND :
        break;
    case ANDHALF :
        break;
    case ANDCONST :
        break;
    case NAND :
        break;
    case NANDHALF :
        break;
    case NANDCONST :
        break;
    case OR :
        break;
    case ORHALF :
        break;
    case ORCONST :
        break;
    case NOR :
        break;
    case NORHALF :
        break;
    case NORCONST :
        break;
    case XOR :
        break;
    case XORHALF :
        break;
    case XORCONST :
        break;
    case XNOR :
        break;
    case XNORHALF :
        break;
    case XNORCONST :
        break;



    }

    if (new_pc > vm->mem_size) {
        printf("instruction overrun");
        exit(-1);
    }
    return new_pc;



}
