#ifndef MEMORY_H
#define MEMORY_H

#define Z33_MEMORY_SIZE 10000

#include <stdint.h>
#include "cpu.h"
#include "instruction.h"
#include <stdbool.h>

typedef enum {
    Cell_Empty,
    Cell_Word,
    Cell_Instruction
} Z33_Cell_Type;

typedef struct {
    Z33_Cell_Type type;

    union {
        Z33_Word word;
        Z33_Instruction instruction;
    } value;
} Z33_Cell;

typedef struct {
    Z33_Cell cells[Z33_MEMORY_SIZE];
} Z33_Memory;

// Functions

void z33_memory_init (Z33_Memory *mem);
bool verify_address (Z33_Address adr);
Z33_Word read_Word_from_memory (  Z33_Memory* mem, Z33_Address ind);
bool write_Word_to_memory (Z33_Memory *mem, Z33_Word word, Z33_Address ind);
Z33_Instruction read_Instruction_from_memory (  Z33_Memory* mem, Z33_Address ind);
bool write_Instruction_to_memory (Z33_Memory* mem, Z33_Instruction inst, Z33_Address ind);
bool verify_empty_cell (  Z33_Memory *mem, Z33_Address ind);

#endif 
