#include "../include/exception.h"

void z33_raise_exception (Z33_Machine *machine,Z33_Exception exception){
    if(machine->memory.cells[200].type==Cell_Instruction){
        printf("Handler existe.\n");
    }
    else{
        printf("Handler n'existe pas\n");
    } // À finir après
    write_Word_to_memory(&machine->memory,machine->cpu.pc,100);
    write_Word_to_memory(&machine->memory,machine->cpu.sr,101);
    write_Word_to_memory(&machine->memory,(Z33_Word)exception,102);
    z33_set_flag(&machine->cpu, SR_S);
    machine->cpu.pc = 200;
}
