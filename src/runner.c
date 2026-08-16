#include "../include/runner.h"

bool z33_step(Z33_Machine *machine) {
    if (machine->cpu.pc >= Z33_MEMORY_SIZE) {
        fprintf(stderr, "Error: PC points outside memory\n");
        return false;
    }

    if (machine->memory.cells[machine->cpu.pc].type == Cell_Empty) {
        machine->running = false;
        return true;
    }

    if (machine->memory.cells[machine->cpu.pc].type != Cell_Instruction) {
        fprintf(stderr, "Error: PC does not point to an instruction\n");
        z33_raise_exception(machine, EX_INVALID_INSTRUCTION);
        return false;
    }

    Z33_Instruction instruction =
        machine->memory.cells[machine->cpu.pc].value.instruction;

    machine->cpu.pc++;

    z33_execute(machine, &instruction);

    return true;
}

bool z33_run(Z33_Machine *machine) {
    while (machine->running) {
        if (!z33_step(machine))
            return false;
    }

    return true;
}
