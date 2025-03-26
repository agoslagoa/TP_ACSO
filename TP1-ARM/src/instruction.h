#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>

/*
 * Defines the structure of a decoded instruction and 
 * the type alias for function pointers that execute instructions.
 */

typedef struct instruction_t {
    uint32_t value;                  // Raw 32-bit instruction
    uint32_t opcode;                 // Decoded opcode
    uint8_t  Rd, Rn, Rm;             // Destination and source registers
    int64_t  immr;                   // Immediate value (signed)
    uint8_t  imms;                   // Immediate mask bits
    uint8_t  shift;                  // Shift type
    uint8_t  cond;                   // Condition code
    void (*operation_func)(struct instruction_t*); // Pointer to handler function
} instruction_t;

typedef void (*FunctionPtr)(instruction_t*);

#endif // INSTRUCTION_H
