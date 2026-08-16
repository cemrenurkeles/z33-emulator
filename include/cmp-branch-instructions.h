#ifndef CMP_BRANCH_INSTRUCTIONS_H
#define CMP_BRANCH_INSTRUCTIONS_H

#include "executor.h"
#include "exception.h"


bool inst_cmp(Z33_Machine *machine, Z33_Instruction *instruction) ;
bool inst_jmp(Z33_Machine *machine, Z33_Instruction *instruction) ;
#endif 
