#ifndef DATA_MOVEMENT_INSTRUCTIONS_H
#define DATA_MOVEMENT_INSTRUCTIONS_H

#include "executor.h"
#include "exception.h"

bool inst_ld (Z33_Machine *machine,  Z33_Instruction *instruction);

bool inst_st(Z33_Machine *machine,  Z33_Instruction *instruction);

bool inst_push(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_pop(Z33_Machine *machine, Z33_Instruction *instruction) ;

#endif

