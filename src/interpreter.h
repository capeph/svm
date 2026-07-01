#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdint.h>
#include "vm.h"

uint64_t interpret(Vm *vm, uint32_t instruction);


#endif
