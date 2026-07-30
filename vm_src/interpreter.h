#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdint.h>
#include "vm.h"

// execute the instruction the current pc points to, return next pc
uint64_t interpret(Vm *vm);


#endif
