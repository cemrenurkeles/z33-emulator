#ifndef CMP_BRANCH_INSTRUCTIONS_H
#define CMP_BRANCH_INSTRUCTIONS_H

#include "executor.h"
#include "exception.h"


bool inst_cmp(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_jmp(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_jeq (Z33_Machine *machine, Z33_Instruction *instruction);

bool inst_jne(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_jlt(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_jge(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_jle(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_jgt(Z33_Machine *machine, Z33_Instruction *instruction) ;


#endif 
