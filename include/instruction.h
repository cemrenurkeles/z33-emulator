#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include "cpu.h"

typedef enum {
    OP_ADD,    // Add
    OP_SUB,    // Subtract
    OP_MUL,    // Multiply
    OP_DIV,    // Divide
    OP_NEG,    // Negate

    OP_AND,    // Bitwise AND
    OP_OR,     // Bitwise OR
    OP_XOR,    // Bitwise XOR
    OP_NOT,    // Bitwise NOT
    OP_SHL,    // Shift left
    OP_SHR,    // Shift right

    OP_CMP,    // Compare

    OP_JMP,    // Unconditional jump
    OP_JEQ,    // Jump if equal
    OP_JNE,    // Jump if not equal
    OP_JLT,    // Jump if less than
    OP_JLE,    // Jump if less or equal
    OP_JGT,    // Jump if greater than
    OP_JGE,    // Jump if greater or equal

    OP_LD,     // Load value
    OP_ST,     // Store value
    OP_SWAP,   // Swap values
    OP_PUSH,   // Push onto stack
    OP_POP,    // Pop from stack

    OP_CALL,   // Call subroutine
    OP_RTN,    // Return from subroutine
    OP_TRAP,   // Trigger trap
    OP_RTI,    // Return from interrupt
    OP_RESET,  // Halt processor
    OP_NOP,    // No operation

    OP_FAS,    // Fetch and set

    OP_IN,     // Read from I/O port
    OP_OUT,    // Write to I/O port

    OP_INVALID // Invalid instruction
} Z33_Opcode;

typedef enum {
    OPERAND_IMM,  // Immediate value
    OPERAND_REG,  // Register
    OPERAND_DIR,  // Direct memory access
    OPERAND_IND,  // Indirect memory access
    OPERAND_IDX   // Indexed memory access
} Z33_Operand_Type;

typedef struct {
    Z33_Operand_Type type;

    union {
        Z33_Word immediate;
        Z33_Register reg;
        Z33_Address address;

        struct {
            Z33_Register reg;
            int32_t offset;
        } indexed;
    } value;

} Z33_Operand;

typedef struct {
    Z33_Opcode opcode;
    Z33_Operand op[2];
    int n_op;
    int line;
} Z33_Instruction;

#endif 
