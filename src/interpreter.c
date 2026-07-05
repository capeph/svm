#include <_string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "vm.h"
#include "opcodes.h"
#include <stdckdint.h>
#include <math.h>

#define OPCODE(instruction) ((instruction >> 22) & 0x3ff)
#define FLAGS(instruction) ((instruction >> 16) & 0x3f)
#define TARGET(instruction) ((instruction >> 8) & 0xff)
#define DATA(instruction) (instruction & 0xff)
#define OFFSET(instruction) ((instruction) & 0xffff)

/* modifiers */

#define SIZE(instruction) ((instruction >> 22 ) & 3) // size is low bits of the opcode


uint64_t interpret(Vm *vm) {
    uint32_t *instrp = (uint32_t *)(vm->memory + vm->pc);
    uint32_t instruction = *instrp;
    int size = SIZE(instruction);
    uint64_t new_pc = vm->pc + size * sizeof(uint32_t);
    if (FLAGS(instruction) & vm->flags)
    {   // conditional skip
        return new_pc;
    }
    int opcode = OPCODE(instruction);
//    printf("Opcode is %d (%x), size is %d\n", opcode, opcode, size);

    switch (opcode) {
    case HALT :
        printf("System halted\n");
        vm->run = false;
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
    case JUMP32 :
    case JUMP64 :
        new_pc = get_const(vm, size);
//        printf("got const %lld\n", new_pc);
        break;
    case JUMPZERO :
        set_flags(vm, get_reg(vm, DATA(instruction)));
        if (vm->flags & ZERO_FLAG) {
            new_pc = get_reg(vm, TARGET(instruction));
        }
        break;
    case JUMPZERO32 :
    case JUMPZERO64 :
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
    case JUMPNOTZERO32 :
    case JUMPNOTZERO64 :
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
    case JUMPPOSITIVE32 :
    case JUMPPOSITIVE64 :
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
    case JUMPNOTPOSITIVE32 :
    case JUMPNOTPOSITIVE64 :
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
    case JUMPNEGATIVE32 :
    case JUMPNEGATIVE64 :
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
    case JUMPNOTNEGATIVE32 :
    case JUMPNOTNEGATIVE64 :
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
    case CALL32 :
    case CALL64 :
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
    case LOADCONSTBYTE :
    {   // use the data register bits for the byte constant
        uint8_t byte = DATA(instruction);
        set_byte(vm, TARGET(instruction), byte);
        break;
    }
    case LOAD :
    {
//        printf("load %x %x\n", TARGET(instruction), DATA(instruction));
        int64_t value = get_reg(vm, DATA(instruction));
//        printf("value = %lld\n", value);
        set_reg(vm, TARGET(instruction), value);
        set_flags(vm, value);
        break;
    }
    case LOAD32 :
    case LOAD64 :
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
        set_flags_double(vm, result);
        break;
    }
    case FLOATTOINT :
    {
        double double_value = get_reg_double(vm, DATA(instruction));
//        printf("got value %f", double_value);
        uint64_t value = (uint64_t)double_value;
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
        int64_t first = get_reg(vm, TARGET(instruction));
        int64_t second = get_reg(vm, DATA(instruction));
        int64_t diff = first - second;
        set_flags(vm, diff);
        break;
    }
    case NEGATE :
    {
        int64_t data = -get_reg(vm, DATA(instruction));
        set_reg(vm, TARGET(instruction), data);
        set_flags(vm, data);
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
            set_flags(vm, dest);
        }
        break;
    }
    case ADD32 :
    case ADD64 :
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
            set_flags(vm, dest);
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
            set_flags(vm, dest);
        }
        break;
    }
    case SUBTRACT32 :
    case SUBTRACT64 :
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
            set_flags(vm, dest);
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
            set_flags(vm, dest);
        }
        break;
    }
    case MULTIPLY32 :
    case MULTIPLY64 :
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
            set_flags(vm, dest);
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
            set_flags(vm, dest);
        }
        break;
    }
    case DIVIDE32 :
        break;
    case DIVIDE64 :
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
            set_flags(vm, dest);
        }
        break;
    }
    case LOADFLOAT :
    {
        double value = get_const_double(vm);
        set_reg_double(vm, TARGET(instruction), value);
        set_flags_double(vm, value);
        break;
    }
    case NEGATEFLOAT :
    {
        double value = -get_reg_double(vm, DATA(instruction));
        set_reg_double(vm, TARGET(instruction), value);
        set_flags_double(vm, value);
        break;
    }
    case ADDFLOAT :
    {
        int target_reg = TARGET(instruction);
        double target = get_reg_double(vm, target_reg);
        double data = get_reg_double(vm, DATA(instruction));
        double dest = target + data;
        if (isinf(dest)) {
            set_overflow(vm);
        }
        else {
            set_reg_double(vm, target_reg, dest);
            set_flags_double(vm, dest);
        }
        break;
    }
    case ADDFLOAT64 :
    {
        int target_reg = TARGET(instruction);
        double data = get_const_double(vm);
        double target = get_reg_double(vm, target_reg);
        double dest = target + data;
        printf("addfloat64 [%f], %f = %f\n", target, data, dest);
        if (isinf(dest)) {
            set_overflow(vm);
        }
        else {
            set_reg_double(vm, target_reg, dest);
            set_flags_double(vm, dest);
        }
        break;
    }
    case SUBTRACTFLOAT :
    {
        int target_reg = TARGET(instruction);
        double target = get_reg_double(vm, target_reg);
        double data = get_reg_double(vm, DATA(instruction));
        double dest = target - data;
        if (isinf(dest)) {
            set_overflow(vm);
        }
        else {
            set_reg_double(vm, target_reg, dest);
            set_flags_double(vm, dest);
        }
        break;
    }
    case SUBTRACTFLOAT64 :
    {
        int target_reg = TARGET(instruction);
        double data = get_const_double(vm);
        double target = get_reg_double(vm, target_reg);
        double dest = target - data;
        if (isinf(dest)) {
            set_overflow(vm);
        }
        else {
            set_reg_double(vm, target_reg, dest);
            set_flags_double(vm, dest);
        }
        break;
    }
    case MULTIPLYFLOAT :
    {
        int target_reg = TARGET(instruction);
        double target = get_reg_double(vm, target_reg);
        double data = get_reg_double(vm, DATA(instruction));
        double dest = target * data;
        if (isinf(dest)) {
            set_overflow(vm);
        }
        else {
            set_reg_double(vm, target_reg, dest);
            set_flags_double(vm, dest);
        }
        break;
    }
    case MULTIPLYFLOAT64 :
    {
        int target_reg = TARGET(instruction);
        double data = get_const_double(vm);
        double target = get_reg(vm, target_reg);
        double dest = target * data;
        if (isinf(dest)) {
            set_overflow(vm);
        }
        else {
            set_reg_double(vm, target_reg, dest);
            set_flags_double(vm, dest);
        }
        break;
    }
    case DIVIDEFLOAT :
     {
        int target_reg = TARGET(instruction);
        double target = get_reg_double(vm, target_reg);
        double data = get_reg_double(vm, DATA(instruction));
        if (data == 0.0) {
            set_overflow(vm);
        }
        else {
            double dest = target / data;
            if (isinf(dest)) {
                set_overflow(vm);
            }
            else {
                set_reg_double(vm, target_reg, dest);
                set_flags_double(vm, dest);
            }
        }
        break;
    }
    case DIVIDEFLOAT64 :
    {
        int target_reg = TARGET(instruction);
        double data = get_const_double(vm);
        double target = get_reg(vm, target_reg);
        double dest = target * data;
        if (data == 0.0) {
            set_overflow(vm);
        }
        else {
            if (isinf(dest)) {
                set_overflow(vm);
            }
            else {
                set_reg_double(vm, target_reg, dest);
                set_flags_double(vm, dest);
            }
        }
        break;
    }
    case NOT :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = ~data;
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case AND :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = target & data;
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case AND32 :
    case AND64 :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = target & data;
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case NAND :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = ~(target & data);
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case NAND32 :
    case NAND64 :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = ~(target & data);
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case OR :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t target = get_reg(vm, target_reg);
        int64_t data = get_reg(vm, DATA(instruction));
        int64_t dest = target | data;
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case OR32 :
    case OR64 :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = target | data;
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case NOR :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = ~(target | data);
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case NOR32 :
    case NOR64 :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = ~(target | data);
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case XOR :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = target ^ data;
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case XOR32 :
    case XOR64 :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = target ^ data;
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case XNOR :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = ~(target ^ data);
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    case XNOR32 :
    case XNOR64 :
    {
        uint64_t target_reg = TARGET(instruction);
        int64_t data = get_const(vm, size);
        int64_t target = get_reg(vm, target_reg);
        int64_t dest = ~(target ^ data);
        set_reg(vm, target_reg, dest);
        set_flags(vm, dest);
        break;
    }
    }

    if (new_pc * sizeof(uint32_t) > vm->mem_size) {
        printf("instruction overrun new_pc=%lld", new_pc);
        exit(-1);
    }
    return new_pc;



}
