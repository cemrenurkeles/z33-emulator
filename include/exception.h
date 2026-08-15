#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "executor.h"

typedef enum {
    EX_DIV_ZERO = 1,
    EX_INVALID_INSTRUCTION = 2,
    EX_PRIVILEGED_INSTRUCTION = 3,
    EX_TRAP = 4,
    EX_INVALID_MEMORY = 5
} Z33_Exception;

void z33_raise_exception (Z33_Machine *machine,Z33_Exception exception);

#endif