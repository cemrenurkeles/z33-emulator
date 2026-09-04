#ifndef EXECUTOR_H
#define EXECUTOR_H

// Comment exécuter cette instruction ?
#include "instruction.h"
#include "memory.h"

#define Z33_SERIAL_BUFFER_SIZE 1024

typedef struct {
    uint8_t receive_buffer[Z33_SERIAL_BUFFER_SIZE];
    size_t receive_head;
    size_t receive_count;
    uint8_t transmit_buffer[Z33_SERIAL_BUFFER_SIZE];
    size_t transmit_head;
    size_t transmit_count;
    bool receive_interrupt_enabled;
    bool interrupt_pending;
    bool interrupt_delivery_pending;
} Z33_SerialController;

typedef struct {
    Z33_CPU cpu;
    Z33_Memory memory;
    Z33_SerialController serial;
    uint64_t cycles;
    bool running;
    bool fatal_error;
    bool exception_raised;
} Z33_Machine;

Z33_Word resolve_operand_value ( Z33_Machine *machine,  Z33_Operand *operand);

Z33_Address resolve_operand_address (Z33_Machine *machine,  Z33_Operand *operand);

void z33_execute(Z33_Machine *machine,  Z33_Instruction *instruction);


#endif
