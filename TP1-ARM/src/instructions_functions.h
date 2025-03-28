#ifndef INSTRUCTION_FUNCTIONS_H
#define INSTRUCTION_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>
#include "decode_core.h"
#include "shell.h"

//
// ───────────────────────────── FLAGS & UTILS ─────────────────────────────
//

void change_flags(int64_t result);
void set_x31();

//
// ───────────────────────────── ARITHMETIC ────────────────────────────────
//

void add_registers(instruction_t* inst);
void sub_registers(instruction_t* inst);
void adds_registers(instruction_t* inst);
void subs_registers(instruction_t* inst);
void add_immediate(instruction_t* inst);
void adds_immediate(instruction_t* inst);
void subs_immediate(instruction_t* inst);
void multiply(instruction_t* inst);

//
// ───────────────────────────── LOGICAL OPS ───────────────────────────────
//

void ands_registers(instruction_t* inst);
void eor_registers(instruction_t* inst);
void orr_registers(instruction_t* inst);

//
// ───────────────────────────── SHIFT ─────────────────────────────────────
//

void logical_shift(instruction_t* inst);

//
// ───────────────────────────── MEMORY ────────────────────────────────────
//

void load_word(instruction_t* inst);
void load_byte(instruction_t* inst);
void load_half(instruction_t* inst);
void store_word(instruction_t* inst);
void store_byte(instruction_t* inst);
void store_half(instruction_t* inst);
void store_n_bits(instruction_t* inst, uint8_t n);

//
// ───────────────────────────── MOVE ──────────────────────────────────────
//

void move_zero(instruction_t* inst);

//
// ───────────────────────────── BRANCH ────────────────────────────────────
//

void branch(instruction_t* inst);
void branch_unconditional(instruction_t* inst);
void branch_register(instruction_t* inst);

void branch_conditional(instruction_t* inst);
void conditional_branch(instruction_t* inst);
void conditional_branch_zero(instruction_t* inst);
void conditional_branch_nonzero(instruction_t* inst);
void compare_and_branch_zero(instruction_t* inst);
void compare_and_branch_nonzero(instruction_t* inst);

#endif // INSTRUCTION_FUNCTIONS_H
