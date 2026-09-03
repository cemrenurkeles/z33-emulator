#include "../include/synchronization-instructions.h"

bool inst_fas(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "Error: fas expects two operands\n");
        return false;
    }

    Z33_Operand *src = &instruction->op[0];
    Z33_Operand *dst = &instruction->op[1];

    if (src->type != OPERAND_DIR && src->type != OPERAND_IND && src->type != OPERAND_IDX) {
        fprintf(stderr, "Error: invalid source operand for fas\n");
        return false;
    }

    if (dst->type != OPERAND_REG) {
        fprintf(stderr, "Error: fas destination must be a register\n");
        return false;
    }

    if (dst->value.reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Address address=resolve_operand_address(machine, src);

    if (!address)
        return false;

    Z33_Word value=read_Word_from_memory(&machine->memory, address);

    if (!value)
        return false;

    if (!z33_set_register(&machine->cpu, dst->value.reg, value))
        return false;

    if (!write_Word_to_memory(&machine->memory, 1, address))
        return false;

    return true;
}
