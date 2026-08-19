#include "../include/executor.h"
#include "../include/exception.h"
#include "../include/arithmetic-instructions.h"
#include "../include/bitwise-instructions.h"
#include "../include/cmp-branch-instructions.h"

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

bool inst_ld (Z33_Machine *machine, Z33_Instruction *instruction){
    if(instruction->n_op!=2){
        fprintf(stderr,"Error operands number incorrect");
        return false;
    }
    if(instruction->op[1].type==OPERAND_REG){
        z33_set_register(&machine->cpu,instruction->op[1].value.reg,resolve_operand_value(machine,&instruction->op[0]));
        return true;
    }
    fprintf(stderr,"ld instruction wrong type for second operand\n");
    return false;
}

bool inst_st(Z33_Machine *machine, Z33_Instruction *instruction)
{
    if (instruction->n_op != 2) {
        fprintf(stderr, "st: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[0].type != OPERAND_REG) {
        fprintf(stderr, "st: first operand must be a register\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_DIR &&
        instruction->op[1].type != OPERAND_IND &&
        instruction->op[1].type != OPERAND_IDX) {
        fprintf(stderr, "st: second operand must be a memory address\n");
        return false;
    }

    Z33_Address address =
        resolve_operand_address(machine, &instruction->op[1]);

    Z33_Word value =
        resolve_operand_value(machine, &instruction->op[0]);

    write_Word_to_memory(&machine->memory, value, address);

    return true;
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
        inst_jeq(machine,instruction);
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
    default:
        fprintf(stderr,"z33_execute : Wrong opcode");
        break;
    }
}

