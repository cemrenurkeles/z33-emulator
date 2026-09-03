#include "../include/i-o-instructions.h"

Z33_Word io_read(Z33_Machine *machine, Z33_Word port) {
    (void)machine;
    (void)port;
    return 0;
}

void io_write(Z33_Machine *machine, Z33_Word port, Z33_Word value) {
    (void)machine;

    if (port == 111) {
        putchar((unsigned char)(value & 0xFF));
        fflush(stdout);
    }
}


bool inst_in(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "Error: in expects two operands\n");
        return false;
    }

    if (!z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Operand *src = &instruction->op[0];
    Z33_Operand *dst = &instruction->op[1];

    if (src->type != OPERAND_DIR && src->type != OPERAND_IND && src->type != OPERAND_IDX) {
        fprintf(stderr, "Error: invalid source operand for in\n");
        return false;
    }

    if (dst->type != OPERAND_REG) {
        fprintf(stderr, "Error: in destination must be a register\n");
        return false;
    }

    Z33_Word port;

    if (src->type == OPERAND_DIR) {
        port = src->value.address;
    } else if (src->type == OPERAND_IND) {
        port = z33_get_register(&machine->cpu, src->value.reg);
    } else {
        port = z33_get_register(&machine->cpu, src->value.indexed.reg) + src->value.indexed.offset;
    }

    Z33_Word value = io_read(machine, port);

    if (!z33_set_register(&machine->cpu, dst->value.reg, value))
        return false;

    return true;
}

bool inst_out(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "Error: out expects two operands\n");
        return false;
    }

    if (!z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Operand *src = &instruction->op[0];
    Z33_Operand *dst = &instruction->op[1];

    if (src->type != OPERAND_IMM && src->type != OPERAND_REG) {
        fprintf(stderr, "Error: invalid source operand for out\n");
        return false;
    }

    if (dst->type != OPERAND_DIR && dst->type != OPERAND_IND && dst->type != OPERAND_IDX) {
        fprintf(stderr, "Error: invalid destination operand for out\n");
        return false;
    }

    Z33_Word value;

    if (src->type == OPERAND_IMM)
        value = src->value.immediate;
    else
        value = z33_get_register(&machine->cpu, src->value.reg);

    Z33_Word port;

    if (dst->type == OPERAND_DIR) {
        port = dst->value.address;
    } else if (dst->type == OPERAND_IND) {
        port = z33_get_register(&machine->cpu, dst->value.reg);
    } else {
        port = z33_get_register(&machine->cpu, dst->value.indexed.reg) + dst->value.indexed.offset;
    }

    io_write(machine, port, value);

    return true;
}
