// Qu'est-ce qu'un CPU Zorglub33 ?


#ifndef CPU_H
#define CPU_H

#include <stdint.h>

// Types de base
typedef int64_t Z33_Word;
typedef uint32_t Z33_Adress;

// Registres identifiables
typedef enum {
    REG_A,
    REG_B,
    REG_PC,
    REG_SP,
    REG_SR
} Z33_Register;

// Flags du SR
#define SR_C (1ULL <<0)
#define SR_Z (1ULL << 1)
#define SR_N (1ULL << 2)
#define SR_O (1ULL<<3)
#define SR_IE  (1ULL << 8)
#define SR_S   (1ULL<< 9)
// Utilisation : cpu->sr |= SR_Z ;

// État du CPU
typedef struct 
{
    Z33_Word a;
    Z33_Word b;
    Z33_Word sr;

    Z33_Adress pc;
    Z33_Adress sp;
} Z33_CPU; 

// Fonctions
void z33_cpu_reset(Z33_CPU *cpu); // initialiser le CPU (=préparer pour la première fois et remettre dans son état initial dans une seule fonction)

Z33_Word z33_get_register(const Z33_CPU *cpu, Z33_Register reg);

int z33_set_register (Z33_CPU *cpu, Z33_Register reg, Z33_Word value);

int z33_get_flag( const Z33_CPU *cpu, Z33_Word flag);

void z33_set_flag(Z33_CPU *cpu, Z33_Word flag);

void z33_clear_flag(Z33_CPU *cpu, Z33_Word flag);

#endif