#include <stdio.h>
#include "../include/memory.h"

void z33_memory_init (Z33_Memory *mem){
    for(int i=0; i<Z33_MEMORY_SIZE; i++){
        mem->cells[i].type=Cell_Empty;
    }
}

bool verify_address (Z33_Address adr){
    return (adr<Z33_MEMORY_SIZE&&adr>0);
}

Z33_Word read_Word_from_memory (const Z33_Memory* mem, Z33_Address ind){
    if(verify_address(ind)) {
        if(mem->cells[ind].type == Cell_Word)
            return mem->cells[ind].value.word;
        else if(mem->cells[ind].type == Cell_Empty)
            return 0;
        else{
            fprintf(stderr,"read_Word_from_memory : Wrong type");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stderr,"read_Word_from_memory : Wrong address");
    exit(EXIT_FAILURE);
}

bool write_Word_to_memory (Z33_Memory *mem, Z33_Word word, Z33_Address ind){
    if(verify_address(ind)) {
        mem->cells[ind].type = Cell_Word;
        mem->cells[ind].value.word = word;
        return true;
    }
    fprintf(stderr,"write_Word_to_memory : Wrong address");
    return false;
}

Z33_Instruction read_Instruction_from_memory (const Z33_Memory* mem, Z33_Address ind){
        if(verify_address(ind)) {
        if(mem->cells[ind].type == Cell_Instruction)
            return mem->cells[ind].value.instruction;
        else{
            fprintf(stderr,"read_Instruction_from_memory : Wrong type");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stderr,"read_Instruction_from_memory : Wrong address");
    exit(EXIT_FAILURE);
}

bool write_Instruction_to_memory (Z33_Memory* mem, Z33_Instruction inst, Z33_Address ind){
    if(verify_address(ind)) {
        mem->cells[ind].type = Cell_Instruction;
        mem->cells[ind].value.instruction = inst;
        return true;
    }
    fprintf(stderr,"write_Instruction_to_memory : Wrong address");
    return false;

}

bool verify_empty_cell (const Z33_Memory *mem, Z33_Address ind){
    if(verify_address(ind)) 
        return  mem->cells[ind].type == Cell_Empty;
    fprintf(stderr,"verify_empty_cell : Wrong address");
    return false;
}
