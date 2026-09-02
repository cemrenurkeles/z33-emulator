#include "../include/stack-instructions.h"

bool inst_push(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "Error: push expects one operand\n");
        return false;
    }

    Z33_Word value = resolve_operand_value(machine, &instruction->op[0]);

    if (machine->cpu.sp == 0) {
        fprintf(stderr, "Error: stack overflow\n");
        return false;
    }

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
        fprintf(stderr, "Error: stack underflow\n");
        return false;
    }

    Z33_Word value = read_Word_from_memory(&machine->memory, machine->cpu.sp);
    machine->cpu.sp++;

    z33_set_register(&machine->cpu, instruction->op[0].value.reg, value);

    return true;
}
