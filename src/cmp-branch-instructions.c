#include "../include/cmp-branch-instructions.h"

bool inst_cmp(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "cmp: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "cmp: second operand must be a register\n");
        return false;
    }
    Z33_Word src = resolve_operand_value(machine,&instruction->op[0]);
    Z33_Word reg = resolve_operand_value(machine,&instruction->op[1]);
    if (src == reg)
        z33_set_flag(&machine->cpu, SR_Z);
    else
        z33_clear_flag(&machine->cpu, SR_Z);

    if (src < reg)
        z33_set_flag(&machine->cpu, SR_N);
    else
        z33_clear_flag(&machine->cpu, SR_N);

    return true;
}


bool inst_jmp(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "jmp: incorrect number of operands\n");
        return false;
    }

    Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);

    if (target < 0 || target >= Z33_MEMORY_SIZE) {
        fprintf(stderr, "jmp: invalid target address\n");
        z33_raise_exception(machine, EX_INVALID_MEMORY);
        return false;
    }

    machine->cpu.pc = (Z33_Address)target;
    return true;
}

