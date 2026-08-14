#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

// Basic Types
typedef int64_t Z33_Word;
typedef uint32_t Z33_Adress;

// Identifiable registers
typedef enum {
    REG_A,
    REG_B,
    REG_PC,
    REG_SP,
    REG_SR
} Z33_Register;

// Status register flags
#define SR_C (1ULL <<0)
#define SR_Z (1ULL << 1)
#define SR_N (1ULL << 2)
#define SR_O (1ULL<<3)
#define SR_IE  (1ULL << 8)
#define SR_S   (1ULL<< 9)
// Usage exemple : cpu->sr |= SR_Z ;

// CPU state
typedef struct 
{
    Z33_Word a;
    Z33_Word b;
    Z33_Word sr;

    Z33_Adress pc;
    Z33_Adress sp;
} Z33_CPU; 

// Functions
void z33_cpu_reset(Z33_CPU *cpu); // Initialize the CPU or reset it to its initial state

Z33_Word z33_get_register(const Z33_CPU *cpu, Z33_Register reg);

bool z33_set_register (Z33_CPU *cpu, Z33_Register reg, Z33_Word value);

bool z33_get_flag( const Z33_CPU *cpu, Z33_Word flag);

void z33_set_flag(Z33_CPU *cpu, Z33_Word flag);

void z33_clear_flag(Z33_CPU *cpu, Z33_Word flag);

#endif