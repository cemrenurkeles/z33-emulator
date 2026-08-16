#include "bitwise-instructions.h"

bool inst_and(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "and: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "and: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);
    Z33_Word dest = z33_get_register(&machine->cpu, dest_reg);
    Z33_Word result = dest & src;

    if (!z33_set_register(&machine->cpu, dest_reg, result)) {
        fprintf(stderr, "and: invalid result for destination register\n");
        return false;
    }

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0) z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    return true;
}

bool inst_or(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "or: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "or: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);
    Z33_Word dest = z33_get_register(&machine->cpu, dest_reg);
    Z33_Word result = dest | src;

    if (!z33_set_register(&machine->cpu, dest_reg, result)) {
        fprintf(stderr, "or: invalid result for destination register\n");
        return false;
    }

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0) z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    return true;
}

bool inst_xor(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "xor: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "xor: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);
    Z33_Word dest = z33_get_register(&machine->cpu, dest_reg);
    Z33_Word result = dest ^ src;

    if (!z33_set_register(&machine->cpu, dest_reg, result)) {
        fprintf(stderr, "xor: invalid result for destination register\n");
        return false;
    }

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0) z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    return true;
}
bool inst_not(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "not: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[0].type != OPERAND_REG) {
        fprintf(stderr, "not: operand must be a register\n");
        return false;
    }

    Z33_Register reg = instruction->op[0].value.reg;

    if (reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word value = z33_get_register(&machine->cpu, reg);
    Z33_Word result = ~value;

    if (!z33_set_register(&machine->cpu, reg, result)) {
        fprintf(stderr, "not: invalid result for destination register\n");
        return false;
    }

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0) z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    return true;
}

bool inst_shl(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "shl: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "shl: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);

    if (src < 0 || src > UINT32_MAX) {
        fprintf(stderr, "shl: invalid shift amount\n");
        z33_raise_exception(machine, EX_INVALID_INSTRUCTION);
        return false;
    }

    uint32_t shift = (uint32_t)src;

    if (shift >= 64) {
        fprintf(stderr, "shl: shift amount exceeds word width\n");
        z33_raise_exception(machine, EX_INVALID_INSTRUCTION);
        return false;
    }

    Z33_Word value = z33_get_register(&machine->cpu, dest_reg);
    Z33_Word result = (Z33_Word)((uint64_t)value << shift);

    if (!z33_set_register(&machine->cpu, dest_reg, result)) {
        fprintf(stderr, "shl: invalid result for destination register\n");
        return false;
    }

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0) z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    return true;
}

bool inst_shr(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "shr: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "shr: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);

    if (src < 0 || src > UINT32_MAX) {
        fprintf(stderr, "shr: invalid shift amount\n");
        z33_raise_exception(machine, EX_INVALID_INSTRUCTION);
        return false;
    }

    uint32_t shift = (uint32_t)src;

    if (shift >= 64) {
        fprintf(stderr, "shr: shift amount exceeds word width\n");
        z33_raise_exception(machine, EX_INVALID_INSTRUCTION);
        return false;
    }

    Z33_Word value = z33_get_register(&machine->cpu, dest_reg);
    Z33_Word result = value >> shift;

    if (!z33_set_register(&machine->cpu, dest_reg, result)) {
        fprintf(stderr, "shr: invalid result for destination register\n");
        return false;
    }

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0) z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    return true;
}
