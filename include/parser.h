#ifndef LOADER_H
#define LOADER_H

#include "executor.h"
#include <string.h>

#define LENGTH_MAX_LINE 256
#define LENGTH_MAX_OPCODE 10

bool parse_file(Z33_Machine *machine, const char *filename);

bool parse_line( char *line, Z33_Instruction *instruction);

bool parse_operand( char *text,Z33_Operand *operand);

bool parse_register( char *text,Z33_Operand *reg);

#endif
