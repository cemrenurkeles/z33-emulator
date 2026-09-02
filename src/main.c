#include <stdio.h>
#include <stdlib.h>
#include "../include/parser.h"
#include "../include/runner.h"

void z33_init_machine(Z33_Machine *machine) {
    z33_cpu_reset(&machine->cpu);
    z33_memory_init(&machine->memory);
    machine->running = true;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.s>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Z33_Machine machine;

    z33_init_machine(&machine);
    machine.running = true;

    printf("Before:\n");
    printf("A = %lld\n", (long long)machine.cpu.a);
    printf("B = %lld\n", (long long)machine.cpu.b);
    printf("PC = %u\n", machine.cpu.pc);
    printf("Z = %d\n", z33_get_flag(&machine.cpu, SR_Z));
    printf("N = %d\n", z33_get_flag(&machine.cpu, SR_N));
printf("SP = %u\n", machine.cpu.sp);

    printf("\nLoading program:\n\n");

    if (!parse_file(&machine, argv[1])) {
        fprintf(stderr, "Failed to load program\n");
        return EXIT_FAILURE;
    }

    printf("\nExecution started:\n\n");

    if (!z33_run(&machine)) {
        fprintf(stderr, "Execution failed\n");
        return EXIT_FAILURE;
    }

    printf("\nAfter:\n");
    printf("A = %lld\n", (long long)machine.cpu.a);
    printf("B = %lld\n", (long long)machine.cpu.b);
    printf("PC = %u\n", machine.cpu.pc);
    printf("Z = %d\n", z33_get_flag(&machine.cpu, SR_Z));
    printf("N = %d\n", z33_get_flag(&machine.cpu, SR_N));
    printf("SP = %u\n", machine.cpu.sp);
    return EXIT_SUCCESS;
}
