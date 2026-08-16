#include "../include/executor.h"
#include "../include/exception.h"
#include <stdint.h>
#include <limits.h>

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


bool inst_add(Z33_Machine *machine, Z33_Instruction *instruction){
    if (instruction->n_op != 2) {
        fprintf(stderr, "add: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "add: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;
    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception( machine, EX_PRIVILEGED_INSTRUCTION );
        return false;
    }

    Z33_Word src =resolve_operand_value(machine,&instruction->op[0]);

    Z33_Word dest =z33_get_register( &machine->cpu,dest_reg );
    uint64_t unsigned_result = (uint64_t)dest + (uint64_t)src;

    Z33_Word result = (Z33_Word)unsigned_result;

    bool overflow =((dest >= 0 && src >= 0 && result < 0) ||(dest < 0 && src < 0 && result >= 0));
    if (overflow) z33_set_flag(&machine->cpu, SR_O);
    else z33_clear_flag(&machine->cpu, SR_O);

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);
    if (result < 0)
        z33_set_flag(&machine->cpu, SR_N);
    else
        z33_clear_flag(&machine->cpu, SR_N);

    if (z33_set_register( &machine->cpu, dest_reg, result ) != true) {
        fprintf(stderr, "add: invalid result for destination register\n");
        return false;
    }

    return true;
}

bool inst_sub( Z33_Machine *machine, Z33_Instruction *instruction){
    if (instruction->n_op != 2) {
        fprintf(stderr, "sub: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "sub: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception( machine ,EX_PRIVILEGED_INSTRUCTION );
        return false;
    }

    Z33_Word src = resolve_operand_value( machine,  &instruction->op[0]        );

    Z33_Word dest =  z33_get_register( &machine->cpu,dest_reg);
    uint64_t unsigned_result = (uint64_t)dest - (uint64_t)src;

    Z33_Word result = (Z33_Word)unsigned_result;
    bool overflow =
        ((dest >= 0 && src < 0 && result < 0) ||
         (dest < 0 && src >= 0 && result >= 0));

    if (overflow)z33_set_flag(&machine->cpu, SR_O);
    else   z33_clear_flag(&machine->cpu, SR_O);

    if (result == 0)  z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0)  z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    if (z33_set_register(&machine->cpu, dest_reg,result) != true) {
        fprintf(stderr,
                "sub: invalid result for destination register\n");

        return false;
    }

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
    default:
        fprintf(stderr,"z33_execute : Wrong opcode");
        break;
    }
}
