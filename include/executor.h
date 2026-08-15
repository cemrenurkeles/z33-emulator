#ifndef EXECUTOR_H
#define EXECUTOR_H

// Comment exécuter cette instruction ?
#include "instruction.h"
#include "memory.h"

typedef struct {
    Z33_CPU cpu;
    Z33_Memory memory;
} Z33_Machine;

Z33_Word resolve_operand_value (const Z33_Machine *machine, const Z33_Operand *operand);

Z33_Address resolve_operand_address (const Z33_Machine *machine, const Z33_Operand *operand);

bool inst_ld (Z33_Machine *machine, const Z33_Instruction *instruction);

void z33_execute(Z33_Machine *machine, const Z33_Instruction *instruction);

#endif