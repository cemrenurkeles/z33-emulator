#ifndef LOADER_H
#define LOADER_H

#include "executor.h"
#include <string.h>
#include <ctype.h>

#define LENGTH_MAX_LINE 256
#define LENGTH_MAX_OPCODE 10

typedef struct {
    const char *mnemonic;
    Z33_Opcode opcode;
    int n_op;
} Z33_OpcodeEntry;


bool parse_file(Z33_Machine *machine, const char *filename);

bool parse_line( Z33_Machine *machine, char *line, Z33_Instruction *instruction);

bool parse_operand(Z33_Machine *machine, char *text,Z33_Operand *operand);

bool parse_register(  char *text,Z33_Operand *reg);

#endif
