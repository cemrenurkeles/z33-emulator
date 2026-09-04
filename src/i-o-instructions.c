#include "../include/i-o-instructions.h"
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define SERIAL_STATUS_PORT 110
#define SERIAL_DATA_PORT 111
#define SERIAL_STATUS_READY (1U << 0)
#define SERIAL_STATUS_TRANSMIT_READY (1U << 1)
#define SERIAL_STATUS_INTERRUPT (1U << 2)

void z33_io_init(Z33_Machine *machine) {
    memset(&machine->serial, 0, sizeof(machine->serial));
}

void z33_serial_receive(Z33_Machine *machine, const uint8_t *bytes, size_t count) {
    Z33_SerialController *serial = &machine->serial;
    for (size_t i = 0; i < count && serial->receive_count < Z33_SERIAL_BUFFER_SIZE; i++) {
        size_t tail = (serial->receive_head + serial->receive_count) % Z33_SERIAL_BUFFER_SIZE;
        serial->receive_buffer[tail] = bytes[i];
        serial->receive_count++;
    }

    if (serial->receive_interrupt_enabled && serial->receive_count != 0 &&
        !serial->interrupt_pending) {
        serial->interrupt_pending = true;
        serial->interrupt_delivery_pending = true;
    }
}

void z33_poll_host_input(Z33_Machine *machine) {
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(STDIN_FILENO, &readable);
    struct timeval timeout = {0, 0};

    int ready = select(STDIN_FILENO + 1, &readable, NULL, NULL, &timeout);
    if (ready <= 0 || !FD_ISSET(STDIN_FILENO, &readable))
        return;

    uint8_t bytes[256];
    ssize_t count = read(STDIN_FILENO, bytes, sizeof(bytes));
    if (count > 0)
        z33_serial_receive(machine, bytes, (size_t)count);
    else if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        perror("serial input");
}

void z33_deliver_pending_interrupt(Z33_Machine *machine) {
    Z33_SerialController *serial = &machine->serial;
    if (serial->interrupt_delivery_pending && z33_get_flag(&machine->cpu, SR_IE)) {
        serial->interrupt_delivery_pending = false;
        z33_raise_exception(machine, EX_HARDWARE_INTERRUPT);
    }
}

Z33_Word io_read(Z33_Machine *machine, Z33_Word port) {
    Z33_SerialController *serial = &machine->serial;
    if (port == SERIAL_STATUS_PORT) {
        Z33_Word status = SERIAL_STATUS_TRANSMIT_READY;
        if (serial->receive_count != 0)
            status |= SERIAL_STATUS_READY;
        if (serial->interrupt_pending)
            status |= SERIAL_STATUS_INTERRUPT;
        serial->interrupt_pending = false;
        return status;
    }
    if (port == SERIAL_DATA_PORT && serial->receive_count != 0) {
        uint8_t byte = serial->receive_buffer[serial->receive_head];
        serial->receive_head = (serial->receive_head + 1) % Z33_SERIAL_BUFFER_SIZE;
        serial->receive_count--;
        return byte;
    }
    return 0;
}

void io_write(Z33_Machine *machine, Z33_Word port, Z33_Word value) {
    Z33_SerialController *serial = &machine->serial;
    if (port == SERIAL_STATUS_PORT) {
        bool was_enabled = serial->receive_interrupt_enabled;
        serial->receive_interrupt_enabled = (value & 1) != 0;
        if (!was_enabled && serial->receive_interrupt_enabled && serial->receive_count != 0 &&
            !serial->interrupt_pending) {
            serial->interrupt_pending = true;
            serial->interrupt_delivery_pending = true;
        }
        return;
    }

    if (port == SERIAL_DATA_PORT) {
        uint8_t byte = (uint8_t)value;
        if (serial->transmit_count < Z33_SERIAL_BUFFER_SIZE) {
            size_t tail = (serial->transmit_head + serial->transmit_count) % Z33_SERIAL_BUFFER_SIZE;
            serial->transmit_buffer[tail] = byte;
            serial->transmit_count++;
        }
        putchar(byte);
        fflush(stdout);
    }
}


bool inst_in(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "Error: in expects two operands\n");
        return false;
    }

    if (!z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Operand *src = &instruction->op[0];
    Z33_Operand *dst = &instruction->op[1];

    if (src->type != OPERAND_DIR && src->type != OPERAND_IND && src->type != OPERAND_IDX) {
        fprintf(stderr, "Error: invalid source operand for in\n");
        return false;
    }

    if (dst->type != OPERAND_REG) {
        fprintf(stderr, "Error: in destination must be a register\n");
        return false;
    }

    Z33_Word port;

    if (src->type == OPERAND_DIR) {
        port = src->value.address;
    } else if (src->type == OPERAND_IND) {
        port = z33_get_register(&machine->cpu, src->value.reg);
    } else {
        port = z33_get_register(&machine->cpu, src->value.indexed.reg) + src->value.indexed.offset;
    }

    Z33_Word value = io_read(machine, port);

    if (!z33_set_register(&machine->cpu, dst->value.reg, value))
        return false;

    return true;
}

bool inst_out(Z33_Machine *machine, Z33_Instruction *instruction) {
    if (instruction->n_op != 2) {
        fprintf(stderr, "Error: out expects two operands\n");
        return false;
    }

    if (!z33_get_flag(&machine->cpu, SR_S)) {
        z33_raise_exception(machine, EX_PRIVILEGED_INSTRUCTION);
        return false;
    }

    Z33_Operand *src = &instruction->op[0];
    Z33_Operand *dst = &instruction->op[1];

    if (src->type != OPERAND_IMM && src->type != OPERAND_REG) {
        fprintf(stderr, "Error: invalid source operand for out\n");
        return false;
    }

    if (dst->type != OPERAND_DIR && dst->type != OPERAND_IND && dst->type != OPERAND_IDX) {
        fprintf(stderr, "Error: invalid destination operand for out\n");
        return false;
    }

    Z33_Word value;

    if (src->type == OPERAND_IMM)
        value = src->value.immediate;
    else
        value = z33_get_register(&machine->cpu, src->value.reg);

    Z33_Word port;

    if (dst->type == OPERAND_DIR) {
        port = dst->value.address;
    } else if (dst->type == OPERAND_IND) {
        port = z33_get_register(&machine->cpu, dst->value.reg);
    } else {
        port = z33_get_register(&machine->cpu, dst->value.indexed.reg) + dst->value.indexed.offset;
    }

    io_write(machine, port, value);

    return true;
}
