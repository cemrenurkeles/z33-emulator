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

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);
    Z33_Word reg = z33_get_register(&machine->cpu, instruction->op[1].value.reg);

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

    machine->cpu.pc = (Z33_Address)target;
    return true;
}

bool inst_jeq(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "jeq: incorrect number of operands\n");
        return false;
    }

    if (z33_get_flag(&machine->cpu, SR_Z)) {
        Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);
        machine->cpu.pc = (Z33_Address)target;
    }

    return true;
}

bool inst_jne(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "jne: incorrect number of operands\n");
        return false;
    }

    if (!z33_get_flag(&machine->cpu, SR_Z)) {
        Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);
        machine->cpu.pc = (Z33_Address)target;
    }

    return true;
}

bool inst_jlt(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "jlt: incorrect number of operands\n");
        return false;
    }

    if (!z33_get_flag(&machine->cpu, SR_Z) &&
        z33_get_flag(&machine->cpu, SR_N)) {
        Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);
        machine->cpu.pc = (Z33_Address)target;
    }

    return true;
}

bool inst_jle(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "jle: incorrect number of operands\n");
        return false;
    }

    if (z33_get_flag(&machine->cpu, SR_Z) ||
        z33_get_flag(&machine->cpu, SR_N)) {
        Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);
        machine->cpu.pc = (Z33_Address)target;
    }

    return true;
}

bool inst_jgt(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "jgt: incorrect number of operands\n");
        return false;
    }

    if (!z33_get_flag(&machine->cpu, SR_Z) &&
        !z33_get_flag(&machine->cpu, SR_N)) {
        Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);
        machine->cpu.pc = (Z33_Address)target;
    }

    return true;
}

bool inst_jge(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "jge: incorrect number of operands\n");
        return false;
    }

    if (z33_get_flag(&machine->cpu, SR_Z) ||
        !z33_get_flag(&machine->cpu, SR_N)) {
        Z33_Word target = resolve_operand_value(machine, &instruction->op[0]);
        machine->cpu.pc = (Z33_Address)target;
    }

    return true;
}
