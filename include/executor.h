#ifndef EXECUTOR_H
#define EXECUTOR_H

// Comment exécuter cette instruction ?
#include "instruction.h"
#include "memory.h"

typedef struct {
    Z33_CPU cpu;
    Z33_Memory memory;
    bool running;
} Z33_Machine;

Z33_Word resolve_operand_value ( Z33_Machine *machine,  Z33_Operand *operand);

Z33_Address resolve_operand_address (Z33_Machine *machine,  Z33_Operand *operand);

void z33_execute(Z33_Machine *machine,  Z33_Instruction *instruction);


#endif
