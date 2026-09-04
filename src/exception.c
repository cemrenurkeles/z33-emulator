#include "../include/exception.h"

void z33_raise_exception(Z33_Machine *machine, Z33_Exception exception) {
    machine->exception_raised = true;
    if (machine->memory.cells[200].type != Cell_Instruction) {
        fprintf(stderr, "Error: no exception handler at address 200\n");
        machine->running = false;
        machine->fatal_error = true;
        return;
    }

    if (!write_Word_to_memory(&machine->memory, machine->cpu.pc, 100)) {
        machine->running = false;
        machine->fatal_error = true;
        return;
    }

    if (!write_Word_to_memory(&machine->memory, machine->cpu.sr, 101)) {
        machine->running = false;
        machine->fatal_error = true;
        return;
    }

    if (!write_Word_to_memory(&machine->memory, (Z33_Word)exception, 102)) {
        machine->running = false;
        machine->fatal_error = true;
        return;
    }

    z33_set_flag(&machine->cpu, SR_S);

    if (exception == EX_HARDWARE_INTERRUPT) {
        z33_clear_flag(&machine->cpu, SR_IE);
    }

    machine->cpu.pc = 200;
    machine->cycles++;
}
