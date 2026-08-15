#include "../include/executor.h"

Z33_Address resolve_operand_address (const Z33_Machine *machine, const Z33_Operand *operand){
    if(operand->type==OPERAND_DIR)
        return operand->value.address;
    if(operand->type==OPERAND_IND)
        return z33_get_register(&machine->cpu,operand->value.reg);
    
    if(operand->type==OPERAND_IDX){
        Z33_Word valeur = z33_get_register(&machine->cpu,operand->value.indexed.reg);
        valeur += operand->value.indexed.offset;
        return valeur;
    }
    fprintf(stderr,"resolve_operand_address : Wrong type");
    exit(EXIT_FAILURE);
}

Z33_Word resolve_operand_value (const Z33_Machine *machine, const Z33_Operand *operand){
    if(operand->type==OPERAND_IMM)
        return operand->value.immediate;
    if(operand->type==OPERAND_REG)
        return z33_get_register(&machine->cpu,operand->value.reg);
    if(operand->type==OPERAND_DIR)
        return read_Word_from_memory(&machine->memory,operand
        ->value.address);
    if(operand->type==OPERAND_IND)
        return read_Word_from_memory(&machine->memory,resolve_operand_address(machine,operand));
    if(operand->type==OPERAND_IDX){
        return read_Word_from_memory(&machine->memory,resolve_operand_address(machine,operand));
    }
    fprintf(stderr,"resolve_operand_value : Wrong type of operand");
    exit(EXIT_FAILURE);
}

bool inst_ld (Z33_Machine *machine, const Z33_Instruction *instruction){
    if(instruction->op[1].type==OPERAND_REG){
        z33_set_register(&machine->cpu,instruction->op[1].value.reg,resolve_operand_value(machine,&instruction->op[0]));
        return true;
    }
    fprintf(stderr,"ld instruction wrong type for second operand\n");
    return false;
}

void z33_execute(Z33_Machine *machine, const Z33_Instruction *instruction){
    switch (instruction->opcode){
    case OP_LD:
        if( inst_ld(machine,instruction))
        printf("Instruction ld executed successfully.");
        break;
    default:
        fprintf(stderr,"z33_execute : Wrong opcode");
        break;
    }

}
