#ifndef CONTROL_FLOW_INSTRUCTIONS_H
#define CONTROL_FLOW_INSTRUCTIONS_H

#include "executor.h"
#include "exception.h"

bool inst_call(Z33_Machine *machine, Z33_Instruction *instruction);

bool inst_rtn(Z33_Machine *machine, Z33_Instruction *instruction);

bool inst_trap(Z33_Machine *machine, Z33_Instruction *instruction);

bool inst_nop(Z33_Machine *machine, Z33_Instruction *instruction);

bool inst_rti(Z33_Machine *machine, Z33_Instruction *instruction);

#endif
