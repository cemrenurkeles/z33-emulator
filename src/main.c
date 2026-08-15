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

    printf("Before execution:\n");
    printf("A = %lld\n", (long long)machine.cpu.a);

    if (!parse_file(&machine, argv[1])) {
        fprintf(stderr, "Failed to parse program\n");
        return 1;
    }

    printf("After execution:\n");
    printf("A = %lld\n", (long long)machine.cpu.a);

    return 0;
}
