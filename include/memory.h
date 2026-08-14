#ifndef MEMORY_H
#define MEMORY_H

#define Z33_MEMORY_SIZE 10000

#include <stdint.h>
#include "cpu.h"
#include "instruction.h"

typedef enum {
    Cell_Empty,
    Cell_Word,
    Cell_Instruction
} Z33_Cell_Type;

typedef struct {
    Z33_Cell_Type type;

    union {
        Z33_Word word;
        Z33_Instruction instruction;
    } value;
} Z33_Cell;

typedef struct {
    Z33_Cell cells[Z33_MEMORY_SIZE];
} Z33_Memory;

#endif 