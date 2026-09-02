#include "../include/data-movement-instructions.h"

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

bool inst_push(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "Error: push expects one operand\n");
        return false;
    }
    if (instruction->op[0].type != OPERAND_IMM && instruction->op[0].type != OPERAND_REG) {
        fprintf(stderr, "Error: push source must be an immediate or register\n");
        return false;
    }
    if (machine->cpu.sp == 0 || machine->cpu.sp > Z33_MEMORY_SIZE) {
        fprintf(stderr, "Error: invalid memory access\n");
        return false;
    }
    Z33_Word value = resolve_operand_value(machine, &instruction->op[0]);
    machine->cpu.sp--;
    write_Word_to_memory(&machine->memory, value, machine->cpu.sp);
    return true;
}

bool inst_pop(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "Error: pop expects one operand\n");
        return false;
    }
    if (instruction->op[0].type != OPERAND_REG) {
        fprintf(stderr, "Error: pop destination must be a register\n");
        return false;
    }
    if (machine->cpu.sp >= Z33_MEMORY_SIZE) {
        fprintf(stderr, "Error: invalid memory access\n");
        return false;
    }
    Z33_Word value = read_Word_from_memory(&machine->memory, machine->cpu.sp);
    if (!z33_set_register(&machine->cpu, instruction->op[0].value.reg, value)) {
        return false;
    }
    machine->cpu.sp++;
    return true;
}
