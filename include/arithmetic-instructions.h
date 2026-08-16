#ifndef ARITHMETIC_INSTRUCTIONS_H
#define ARITHMETIC_INSTRUCTIONS_H

#include <stdint.h>
#include <limits.h>
#include "executor.h"

bool inst_add(Z33_Machine *machine, Z33_Instruction *instruction);

bool inst_sub( Z33_Machine *machine, Z33_Instruction *instruction);

bool inst_mul(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_div(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_neg(Z33_Machine *machine, Z33_Instruction *instruction) ;

#endif
