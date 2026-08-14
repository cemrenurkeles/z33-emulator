#include <stdio.h>
#include <stdlib.h>
#include "../include/cpu.h"

void z33_cpu_reset(Z33_CPU *cpu){
    cpu->a= 0;
    cpu->b = 0;
    cpu->pc= 1000;
    cpu->sp = 10000;
    cpu->sr = SR_S;
}

Z33_Word z33_get_register(const Z33_CPU *cpu, Z33_Register reg){
   switch (reg)
   {
   case REG_A:
       return cpu->a;
   case REG_B :
        return cpu->b;
   case REG_PC :
        return (Z33_Word)cpu->pc;
   case REG_SP:
        return  (Z33_Word)cpu->sp;
   case REG_SR :
        return (Z33_Word)cpu->sr;
   default:
    fprintf(stderr,"z33_get_register : Error register type incorrect ");
    exit(EXIT_FAILURE);
   }
}
// Renvoie -1 en cas d'erreur et 0 en cas de succès
int z33_set_register (Z33_CPU *cpu, Z33_Register reg, Z33_Word value){
       switch (reg)
   {
   case REG_A:
       cpu->a=value;
       break;
   case REG_B :
        cpu->b=value;
        break;
   case REG_PC :
        if (value>=0&&value<=UINT32_MAX)
        cpu->pc = (uint32_t)value;
        else{
            fprintf(stderr,"z33_set_register : Error register value incorrect");
            return -1;
        }
        break;
   case REG_SP:
        if (value>=0&&value<=UINT32_MAX)
        cpu->sp = (uint32_t)value;
        else{
            fprintf(stderr,"z33_set_register : Error sp register value incorrect");
            return -1;
        }
        break;
   case REG_SR :
        cpu->sr =value;
        break;
   default:
    fprintf(stderr,"z33_set_register : Error register type incorrect ");
    return -1;
   }
   return 0;
}

int z33_get_flag( const Z33_CPU *cpu, Z33_Word flag){
    return (cpu->sr & flag) != 0;
}

void z33_set_flag(Z33_CPU *cpu, Z33_Word flag){
    cpu->sr |= flag;
}

void z33_clear_flag(Z33_CPU *cpu, Z33_Word flag){
    cpu->sr &= ~flag;
}