#ifndef DECODE_CORE_H
#define DECODE_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "shell.h"

// ───────────────────────────────────── INSTRUCTION STRUCT ─────
typedef struct instruction_t {
    uint32_t value;                  // Raw 32-bit instruction
    uint32_t opcode;                 // Decoded opcode
    uint8_t  Rd, Rn, Rm;             // Destination and source registers
    int64_t  immr;                   // Immediate value (signed)
    uint8_t  imms;                   // Immediate mask bits
    uint8_t  shift;                  // Shift amount
    uint8_t  cond;                   // Condition code (for B.cond)
    void (*operation_func)(struct instruction_t*); // Function to execute the instruction
} instruction_t;

typedef void (*FunctionPtr)(instruction_t*);

// ───────────────────────────────────── MASK UTILITIES ─────
uint32_t create_mask(uint8_t length, uint8_t shift);
uint32_t extract_bits(uint32_t instr, uint8_t length, uint8_t shift);
uint32_t get_field(instruction_t* inst, uint8_t length, uint8_t shift);

// ───────────────────────────────────── DECODE & EXECUTE ─────
bool decode(instruction_t* inst);
void execute(instruction_t* inst);

#endif // DECODE_CORE_H
