#include "../include/exception.h"
#include "../include/arithmetic-instructions.h"

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

bool inst_mul(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "mul: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "mul: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);
    Z33_Word dest = z33_get_register(&machine->cpu, dest_reg);

    uint64_t unsigned_result = (uint64_t)dest * (uint64_t)src;
    Z33_Word result = (Z33_Word)unsigned_result;

    bool overflow = false;

    if (src != 0) {
        if (src == -1)
            overflow = (dest == INT64_MIN);
        else if (dest == -1)
            overflow = (src == INT64_MIN);
        else
            overflow = (result / src != dest);
    }

    if (z33_set_register(&machine->cpu, dest_reg, result) != true) {
        fprintf(stderr, "mul: invalid result for destination register\n");
        return false;
    }

    if (overflow)
        z33_set_flag(&machine->cpu, SR_O);
    else
        z33_clear_flag(&machine->cpu, SR_O);

    if (result == 0)
        z33_set_flag(&machine->cpu, SR_Z);
    else
        z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0)
        z33_set_flag(&machine->cpu, SR_N);
    else
        z33_clear_flag(&machine->cpu, SR_N);

    return true;
}

bool inst_div(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "div: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[1].type != OPERAND_REG) {
        fprintf(stderr, "div: second operand must be a register\n");
        return false;
    }

    Z33_Register dest_reg = instruction->op[1].value.reg;

    if (dest_reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word src = resolve_operand_value(machine, &instruction->op[0]);
    Z33_Word dest = z33_get_register(&machine->cpu, dest_reg);

    if (src == 0) {
        z33_raise_exception(machine, EX_DIV_ZERO);
        return false;
    }

    if (dest == INT64_MIN && src == -1) {
        fprintf(stderr, "div: signed division overflow\n");
        return false;
    }

    Z33_Word result = dest / src;

    if (z33_set_register(&machine->cpu, dest_reg, result) != true) {
        fprintf(stderr, "div: invalid result for destination register\n");
        return false;
    }

    if (result == 0)
        z33_set_flag(&machine->cpu, SR_Z);
    else
        z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0)
        z33_set_flag(&machine->cpu, SR_N);
    else
        z33_clear_flag(&machine->cpu, SR_N);

    return true;
}

bool inst_neg(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 1) {
        fprintf(stderr, "neg: incorrect number of operands\n");
        return false;
    }

    if (instruction->op[0].type != OPERAND_REG) {
        fprintf(stderr, "neg: operand must be a register\n");
        return false;
    }

    Z33_Register reg = instruction->op[0].value.reg;

    if (reg == REG_SR && !z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Word value = z33_get_register(&machine->cpu, reg);

    bool overflow = (value == INT64_MIN);

    uint64_t unsigned_result = 0ULL - (uint64_t)value;
    Z33_Word result = (Z33_Word)unsigned_result;

    if (!z33_set_register(&machine->cpu, reg, result)) {
        fprintf(stderr, "neg: invalid result for destination register\n");
        return false;
    }

    if (overflow) z33_set_flag(&machine->cpu, SR_O);
    else z33_clear_flag(&machine->cpu, SR_O);

    if (result == 0) z33_set_flag(&machine->cpu, SR_Z);
    else z33_clear_flag(&machine->cpu, SR_Z);

    if (result < 0) z33_set_flag(&machine->cpu, SR_N);
    else z33_clear_flag(&machine->cpu, SR_N);

    return true;
}
