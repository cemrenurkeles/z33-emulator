#include <stdio.h>

#include "../include/parser.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.s>\n", argv[0]);
        return 1;
    }

    Z33_Machine machine;

    z33_cpu_reset(&machine.cpu);
    z33_memory_init(&machine.memory);

    /*
     * Prepare memory/register values for addressing mode tests.
     */

    // Used by direct addressing: [500]
    write_Word_to_memory(&machine.memory, 42, 500);

    // Used by indirect addressing: [%b]
    // B = 600, memory[600] = 84
    z33_set_register(&machine.cpu, REG_B, 600);
    write_Word_to_memory(&machine.memory, 84, 600);

    // Used by indexed addressing: [%b+5]
    // B = 600, memory[605] = 126
    write_Word_to_memory(&machine.memory, 126, 605);

    printf("Before:\n");
    printf("A = %lld\n", (long long)machine.cpu.a);
    printf("B = %lld\n", (long long)machine.cpu.b);

    if (!parse_file(&machine, argv[1])) {
        return 1;
    }

    printf("\nAfter:\n");
    printf("A = %lld\n", (long long)machine.cpu.a);
    printf("B = %lld\n", (long long)machine.cpu.b);

    return 0;
}
