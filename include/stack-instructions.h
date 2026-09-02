#ifndef STACK_INSTRUCTIONS_H
#define STACK_INSTRUCTIONS_H

#include "executor.h"
#include "exception.h"

bool inst_push(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_pop(Z33_Machine *machine, Z33_Instruction *instruction) ;

#endif

