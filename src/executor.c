#include "../include/executor.h"
#include "../include/exception.h"
#include "../include/arithmetic-instructions.h"
#include "../include/bitwise-instructions.h"
#include "../include/cmp-branch-instructions.h"
#include "../include/data-movement-instructions.h"
#include "../include/control-flow-instructions.h"

Z33_Address resolve_operand_address (Z33_Machine *machine, Z33_Operand *operand){
    Z33_Word valeur = -1;
    if(operand->type==OPERAND_DIR)
        valeur = operand->value.address;
    if(operand->type==OPERAND_IND)
        valeur = z33_get_register(&machine->cpu,operand->value.reg);
    
    if(operand->type==OPERAND_IDX){
        valeur = z33_get_register(&machine->cpu,operand->value.indexed.reg);
        valeur += operand->value.indexed.offset;

    }
    if (valeur<0||valeur>=Z33_MEMORY_SIZE){
        fprintf(stderr,"Error : Adress must be [0,%d].",Z33_MEMORY_SIZE);
        z33_raise_exception(machine, EX_INVALID_MEMORY);
    }
    return (Z33_Address) valeur;
}

Z33_Word resolve_operand_value (Z33_Machine *machine, Z33_Operand *operand){
    if(operand->type==OPERAND_IMM)
        return operand->value.immediate;

    if(operand->type==OPERAND_REG)
        return z33_get_register(&machine->cpu,operand->value.reg);

    if (operand->type == OPERAND_DIR ||
        operand->type == OPERAND_IND ||
        operand->type == OPERAND_IDX) {

        return read_Word_from_memory(
            &machine->memory,
            resolve_operand_address(machine, operand)
        );
    }
    fprintf(stderr,"resolve_operand_value : Wrong type of operand");
    exit(EXIT_FAILURE);
}




void z33_execute(Z33_Machine *machine, Z33_Instruction *instruction){
    switch (instruction->opcode){
    case OP_LD:
         inst_ld(machine,instruction);
        break;
    case OP_ST:
        inst_st(machine, instruction);
        break;
    case OP_ADD:
        inst_add(machine,instruction);
        break;
    case OP_SUB:
        inst_sub(machine,instruction);
        break;
    case OP_MUL:
        inst_mul(machine,instruction);
        break;
    case OP_DIV:
        inst_div(machine,instruction);
        break;
    case OP_NEG:
        inst_neg(machine,instruction);
        break;
    case OP_AND:
        inst_and(machine,instruction);
        break;
    case OP_OR:
        inst_or(machine,instruction);
        break;
    case OP_XOR:
        inst_xor(machine,instruction);
        break;
    case OP_NOT:
        inst_not(machine, instruction);
        break;
    case OP_SHL:
        inst_shl(machine,instruction);
        break;
    case OP_SHR:
        inst_shr(machine,instruction);
        break;
    case OP_CMP:
        inst_cmp(machine,instruction);
        break;
    case OP_JMP:
        inst_jmp(machine,instruction);
        break;
    case OP_JEQ:
        inst_jeq(machine,instruction);
        break;
    case OP_JNE:
        inst_jne(machine,instruction);
        break;
    case OP_JLT:
        inst_jlt(machine,instruction);
        break;
    case OP_JGE:
        inst_jge(machine,instruction);
        break;
    case OP_JLE:
        inst_jle(machine,instruction);
        break;
    case OP_JGT:
        inst_jgt(machine,instruction);
        break;
    case OP_POP:
        inst_pop(machine,instruction);
        break;
    case OP_PUSH:
        inst_push(machine,instruction);
        break;
    case OP_SWAP:
        inst_swap(machine,instruction);
        break;
    case OP_CALL:
        inst_call(machine,instruction);
        break;
    case OP_RTN:
        inst_rtn(machine,instruction);
        break;
    case OP_TRAP:
        inst_trap(machine,instruction);
        break;
    case OP_NOP:
        inst_nop(machine,instruction);
        break;
    default:
        fprintf(stderr,"z33_execute : Wrong opcode");
        exit(EXIT_FAILURE);
    }
}

