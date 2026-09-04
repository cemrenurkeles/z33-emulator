#include "../include/runner.h"
#include "../include/i-o-instructions.h"
#include "../include/parser.h"

static uint64_t operand_cost(const Z33_Operand *operand) {
    return operand->type == OPERAND_DIR || operand->type == OPERAND_IND ||
           operand->type == OPERAND_IDX;
}

bool z33_step(Z33_Machine *machine) {
    machine->exception_raised = false;
    if (machine->cpu.pc >= Z33_MEMORY_SIZE) {
        fprintf(stderr, "Error: PC points outside memory\n");
        machine->fatal_error = true;
        return false;
    }

    Z33_Cell *cell = &machine->memory.cells[machine->cpu.pc];
    machine->cpu.pc++;

    if (cell->type != Cell_Instruction) {
        fprintf(stderr, "Error: PC does not point to an instruction\n");
        machine->cycles++;
        z33_raise_exception(machine, EX_INVALID_INSTRUCTION);
        return !machine->fatal_error;
    }

    Z33_Instruction instruction = cell->value.instruction;
    
    fprintf(stderr,"%d  ", instruction.line);
    print_instruction(&instruction);
    fprintf(stderr,"\n");
    
    z33_execute(machine, &instruction);
    machine->cycles++;
    if (instruction.n_op >= 1)
        machine->cycles += operand_cost(&instruction.op[0]);
    if (instruction.n_op == 2)
        machine->cycles += operand_cost(&instruction.op[1]);

    if (machine->running && !machine->exception_raised) {
        z33_poll_host_input(machine);
        z33_deliver_pending_interrupt(machine);
    }

    return !machine->fatal_error;
}

bool z33_run(Z33_Machine *machine) {
    while (machine->running) {
        if (!z33_step(machine))
            return false;
    }

    return !machine->fatal_error;
}
