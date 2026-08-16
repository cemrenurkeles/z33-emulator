#ifndef BITWISE_INSTRUCTIONS_H
#define BITWISE_INSTRUCTIONS_H

#include <stdint.h>
#include <limits.h>
#include "executor.h"
#include "exception.h"

bool inst_and(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_or(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_xor(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_not(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_shl(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_shr(Z33_Machine *machine, Z33_Instruction *instruction) ;

#endif
