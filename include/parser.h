#ifndef LOADER_H
#define LOADER_H

#include "executor.h"
#include <string.h>
#include <ctype.h>

#define LENGTH_MAX_LINE 256
#define LENGTH_MAX_OPCODE 10

typedef struct {
      char *mnemonic;
    Z33_Opcode opcode;
    int n_op;
} Z33_OpcodeEntry;

#define LENGTH_MAX_LABEL 256
#define MAX_LABELS 256

typedef struct {
    char name[LENGTH_MAX_LABEL];
    Z33_Address address;
} Z33_Label;
char *trim(char *str);
bool parse_addr_directive(char *line, Z33_Address *address) ;
bool parse_file(Z33_Machine *machine, char *filename);
bool is_string_directive(char *line) ;
bool get_string_length(char *line, size_t *length) ;
bool get_string_length(char *line, size_t *length) ;
bool parse_line(Z33_Machine *machine, char *line, Z33_Instruction *instruction, const Z33_Label *labels, size_t label_count) ;

bool parse_operand(Z33_Machine *machine, char *text, Z33_Operand *operand, const Z33_Label *labels, size_t label_count) ;

bool parse_register(  char *text,Z33_Operand *reg);
void print_operand(const Z33_Operand *operand) ;

void print_instruction(const Z33_Instruction *instruction) ;
#endif
