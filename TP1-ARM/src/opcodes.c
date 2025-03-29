#include "decode_core.h"
#include "instructions_functions.h"

// ────── LOAD/STORE INSTRUCTIONS ────────────────────────────────────────────
uint32_t OPCODE_LOAD_STORE[] = {
    0b11111000010, // LDUR   (load 64 bits)
    0b00111000010, // LDURB  (load 8 bits)
    0b01111000010, // LDURH  (load 16 bits)
    0b11111000000, // STUR   (store 64 bits)
    0b00111000000, // STURB  (store 8 bits)
    0b01111000000  // STURH  (store 16 bits)
};

FunctionPtr OPCODE_LOAD_STORE_FUNCS[] = {
    load_word, load_byte, load_half, store_word, store_byte, store_half
};

// ────── LOGICAL REGISTER INSTRUCTIONS ──────────────────────────────────────
uint32_t OPCODE_LOGIC[] = {
    0b11101010000, // ANDS
    0b11001010000, // EOR
    0b10101010000  // ORR
};

FunctionPtr OPCODE_LOGIC_FUNCS[] = {
    ands_registers, eor_registers, orr_registers
};

// ────── ARITHMETIC REGISTER INSTRUCTIONS ───────────────────────────────────
uint32_t OPCODE_ARITH_REG[] = {
    0b10101011000, // ADDS
    0b11101011000, // SUBS
    0b10011011000, // MUL
    0b10001011000  // ADD
};

FunctionPtr OPCODE_ARITH_REG_FUNCS[] = {
    adds_registers, subs_registers, multiply, add_registers
};

// ────── ARITHMETIC IMMEDIATE INSTRUCTIONS ──────────────────────────────────
uint32_t OPCODE_ARITH_IMM[] = {
    0b1011000100, // ADDS (imm)
    0b1111000100, // SUBS (imm)
    0b1001000100  // ADD  (imm)
};

FunctionPtr OPCODE_ARITH_IMM_FUNCS[] = {
    adds_immediate, subs_immediate, add_immediate
};

// ────── SHIFT INSTRUCTIONS ─────────────────────────────────────────────────
uint32_t OPCODE_SHIFT[] = {
    0b1101001101 // LSL, LSR
};

FunctionPtr OPCODE_SHIFT_FUNCS[] = {
    logical_shift
};

// ────── BRANCH INSTRUCTIONS ────────────────────────────────────────────────
uint32_t OPCODE_BRANCH[] = {
    0b000101 // B (unconditional)
};

FunctionPtr OPCODE_BRANCH_FUNCS[] = {
    branch_unconditional
};

// ────── BRANCH TO REGISTER ─────────────────────────────────────────────────
uint32_t OPCODE_BRANCH_REG[] = {
    0b1101011000011111000000 // BR
};

FunctionPtr OPCODE_BRANCH_REG_FUNCS[] = {
    branch_register
};

// ────── BRANCH CONDITIONAL ─────────────────────────────────────────────────
uint32_t OPCODE_BRANCH_COND[] = {
    0b01010100 // B.cond
};

FunctionPtr OPCODE_BRANCH_COND_FUNCS[] = {
    branch_conditional
};

// ────── MOVE IMMEDIATE INSTRUCTIONS ────────────────────────────────────────
uint32_t OPCODE_MOVE_IMM[] = {
    0b11010010100 // MOVZ
};

FunctionPtr OPCODE_MOVE_IMM_FUNCS[] = {
    move_zero
};

// ────── COMPARE AND BRANCH INSTRUCTIONS ────────────────────────────────────
uint32_t OPCODE_CMP_BRANCH[] = {
    0b10110100, // CBZ
    0b10110101  // CBNZ
};

FunctionPtr OPCODE_CMP_BRANCH_FUNCS[] = {
    compare_and_branch_zero, compare_and_branch_nonzero
};

