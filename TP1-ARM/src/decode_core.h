#ifndef DECODE_CORE_H
#define DECODE_CORE_H

#include <stdint.h>
#include "shell.h"
#include <stdbool.h>  

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


/*
 * Provides functions for decoding ARMv8 instructions and
 * executing them using function pointers mapped to opcodes.
 */

//
// ──────────────────────────────────────────────────────── MASK UTILITIES ─────
//

uint32_t generate_mask(uint8_t len, uint8_t move_left);
uint32_t apply_mask(instruction_t* inst, uint32_t mask, uint8_t move_right);
uint32_t get_values(instruction_t* inst, uint8_t move, uint16_t amount_ones);

//
// ─────────────────────────────────────────────────────── FIELD EXTRACTION ─────
//

void extract_base_register(instruction_t* inst);
void extract_destination_register(instruction_t* inst);
void extract_second_register(instruction_t* inst);
void extract_signed_immediate(instruction_t* inst, uint8_t move_left, uint8_t len);
void extract_unsigned_immediate(instruction_t* inst, uint8_t move_left, uint8_t len);
void extract_shift_amount(instruction_t* inst);
void apply_branch_shift(instruction_t* inst);

//
// ──────────────────────────────────────────────────────── INSTRUCTION FORMATS ─────
//

void format_load_store(instruction_t* inst);
void format_three_registers(instruction_t* inst);
void format_immediate_arithmetic(instruction_t* inst);
void format_shift(instruction_t* inst);
void format_unconditional_branch(instruction_t* inst);
void format_branch_register(instruction_t* inst);
void format_conditional_branch(instruction_t* inst);
void format_move_immediate(instruction_t* inst);
void format_compare_and_branch(instruction_t* inst);

//
// ──────────────────────────────────────────────────────── DECODING & EXECUTION ─────
//

void set_parameters(instruction_t* inst, uint8_t opcode_type);
bool set_instruction(instruction_t* inst, int32_t possible_opcodes[], uint8_t amount_op, FunctionPtr* type_functions, uint8_t type);
bool decode(instruction_t* inst);
void execute(instruction_t* inst);

#endif // DECODE_CORE_H