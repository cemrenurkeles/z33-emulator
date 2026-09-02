#include "../include/control-flow-instructions.h"

bool inst_call(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "call: incorrect number of operands\n");
        return false;
    }

    Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);

    if (target < 0 || target >= Z33_MEMORY_SIZE) {
        fprintf(stderr, "call: invalid target address\n");
        z33_raise_exception(machine, EX_INVALID_MEMORY);
        return false;
    }

    if (machine->cpu.sp == 0 || machine->cpu.sp > Z33_MEMORY_SIZE) {
        fprintf(stderr, "call: invalid stack address\n");
        z33_raise_exception(machine, EX_INVALID_MEMORY);
        return false;
    }

    machine->cpu.sp--;

    if (!write_Word_to_memory(&machine->memory, (Z33_Word)machine->cpu.pc, machine->cpu.sp)) {
        return false;
    }

    machine->cpu.pc = (Z33_Address)target;

    return true;
}

bool inst_rtn(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 0) {
        fprintf(stderr, "rtn: incorrect number of operands\n");
        return false;
    }

    if (machine->cpu.sp >= Z33_MEMORY_SIZE) {
        fprintf(stderr, "rtn: invalid stack address\n");
        z33_raise_exception(machine, EX_INVALID_MEMORY);
        return false;
    }

    Z33_Word return_address = read_Word_from_memory(&machine->memory, machine->cpu.sp);

    machine->cpu.pc = (Z33_Address)return_address;
    machine->cpu.sp++;

    return true;
}

