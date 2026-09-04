#ifndef I_O_INSTRUCTIONS_H
#define I_O_INSTRUCTIONS_H

#include "executor.h"
#include "exception.h"

void z33_io_init(Z33_Machine *machine);
void z33_serial_receive(Z33_Machine *machine, const uint8_t *bytes, size_t count);
void z33_poll_host_input(Z33_Machine *machine);
void z33_deliver_pending_interrupt(Z33_Machine *machine);

bool inst_out(Z33_Machine *machine, Z33_Instruction *instruction) ;

bool inst_in(Z33_Machine *machine, Z33_Instruction *instruction);


#endif
