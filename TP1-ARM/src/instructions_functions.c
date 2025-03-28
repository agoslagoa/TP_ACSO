#include "instructions_functions.h"
#include "decode_core.h"
#include <stdbool.h>

//
// ──────────────────────────────────────────────────────────────── FLAGS & UTILS ──────
//

void set_x31() {
    // Ensures register X31 is always zero
    NEXT_STATE.REGS[31] = 0;
}

void change_flags(int64_t result) {
    // Sets the Zero and Negative flags based on the result
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result < 0);
}


//
// ──────────────────────────────────────────────────────────────── BASIC OPERATIONS ──────
//

int64_t op_add(int64_t a, int64_t b) {
    return a + b;
}

int64_t op_sub(int64_t a, int64_t b) {
    return a - b;
}

void arithmetic_operation(instruction_t* inst, int64_t (*operation)(int64_t, int64_t)) {
    int64_t operand1 = CURRENT_STATE.REGS[inst->Rn];
    int64_t operand2 = (inst->shift == 0b01) ? (inst->immr << 12) : (
                        inst->shift == 0b00 ? inst->immr : CURRENT_STATE.REGS[inst->Rm]);

    int64_t result = operation(operand1, operand2);

    NEXT_STATE.REGS[inst->Rd] = result;
    change_flags(result);
    set_x31();
}

//
// ──────────────────────────────────────────────────────────────── ARITHMETIC ──────
//

void adds_registers(instruction_t* inst)       { arithmetic_operation(inst, op_add); }
void subs_registers(instruction_t* inst)       { arithmetic_operation(inst, op_sub); }
void adds_immediate(instruction_t* inst)       { arithmetic_operation(inst, op_add); }
void subs_immediate(instruction_t* inst)       { arithmetic_operation(inst, op_sub); }

void add_registers(instruction_t* inst) {
    int64_t operand1 = CURRENT_STATE.REGS[inst->Rn];
    int64_t operand2 = CURRENT_STATE.REGS[inst->Rm];
    NEXT_STATE.REGS[inst->Rd] = operand1 + operand2;
    set_x31();
}

void sub_registers(instruction_t* inst) {
    int64_t operand1 = CURRENT_STATE.REGS[inst->Rn];
    int64_t operand2 = CURRENT_STATE.REGS[inst->Rm];
    NEXT_STATE.REGS[inst->Rd] = operand1 - operand2;
    set_x31();
}

void add_immediate(instruction_t* inst) {
    int64_t operand1 = CURRENT_STATE.REGS[inst->Rn];
    int64_t operand2 = (inst->shift == 0b01) ? (inst->immr << 12) : inst->immr;
    NEXT_STATE.REGS[inst->Rd] = operand1 + operand2;
    set_x31();
}

void multiply(instruction_t* inst) {
    NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rn] * NEXT_STATE.REGS[inst->Rm];
    set_x31();
}

//
// ──────────────────────────────────────────────────────────────── LOGIC ──────
//

void ands_registers(instruction_t* inst) {
    NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rm] & NEXT_STATE.REGS[inst->Rn];
    change_flags(NEXT_STATE.REGS[inst->Rd]);
    set_x31();
}

void eor_registers(instruction_t* inst) {
    NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rm] ^ NEXT_STATE.REGS[inst->Rn];
    set_x31();
}

void orr_registers(instruction_t* inst) {
    NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rm] | NEXT_STATE.REGS[inst->Rn];
    set_x31();
}

//
// ──────────────────────────────────────────────────────────────── SHIFT ──────
//


void shift(instruction_t* inst, uint32_t shift_type) {
    uint32_t shift = (uint32_t) inst->immr;

    if (shift_type == 1) {  // LSR
        NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rn] >> shift;
    } else if (shift_type == 0) {  // LSL
        NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rn] << shift;
    }

    set_x31();
}


void logical_shift(instruction_t* inst) {
    shift(inst, inst->imms);
}

//
// ──────────────────────────────────────────────────────────────── MEMORY ──────
//

void load_word(instruction_t* inst) {
    uint64_t address = NEXT_STATE.REGS[inst->Rn] + inst->immr;
    uint64_t val_lo = (uint64_t) mem_read_32(address);
    uint64_t val_hi = (uint64_t) mem_read_32(address + 4) << 32;

    NEXT_STATE.REGS[inst->Rd] = val_lo + val_hi;
    set_x31();
}

void load_byte(instruction_t* inst) {
    uint64_t address = NEXT_STATE.REGS[inst->Rn] + inst->immr;
    NEXT_STATE.REGS[inst->Rd] = mem_read_32(address) & 0xFF;
    set_x31();
}

void load_half(instruction_t* inst) {
    uint64_t address = NEXT_STATE.REGS[inst->Rn] + inst->immr;
    NEXT_STATE.REGS[inst->Rd] = mem_read_32(address) & 0xFFFF;
    set_x31();
}

void store_word(instruction_t* inst) {
    uint64_t address = NEXT_STATE.REGS[inst->Rn] + inst->immr;
    uint64_t value = NEXT_STATE.REGS[inst->Rd];

    uint64_t mask_lo = (uint64_t) generate_mask(32, 0);
    uint64_t mask_hi = mask_lo << 32;

    mem_write_32(address,     (uint32_t)(value & mask_lo));
    mem_write_32(address + 4, (uint32_t)((value & mask_hi) >> 32));
}

void store_n_bits(instruction_t* inst, uint8_t n) {
    uint64_t address = NEXT_STATE.REGS[inst->Rn] + inst->immr;
    uint32_t prev = mem_read_32(address);

    uint32_t mask_old = generate_mask(32 - n, n);
    uint64_t mask_reg = (uint64_t) generate_mask(n, 0);
    uint32_t new_bits = (uint32_t)(NEXT_STATE.REGS[inst->Rd] & mask_reg);

    prev = (prev & mask_old) + new_bits;
    mem_write_32(address, prev);
}

void store_byte(instruction_t* inst) {
    store_n_bits(inst, 8);
}

void store_half(instruction_t* inst) {
    store_n_bits(inst, 16);
}

//
// ──────────────────────────────────────────────────────────────── BRANCHES ──────
//

void branch(instruction_t* inst) {
    NEXT_STATE.PC += inst->immr - 4;
}

void branch_unconditional(instruction_t* inst) {
    NEXT_STATE.PC += inst->immr - 4;
}

void branch_register(instruction_t* inst) {
    NEXT_STATE.PC = NEXT_STATE.REGS[inst->Rn];
}

void conditional_branch(instruction_t* inst) {
    bool cond_x = false;

    switch (inst->cond) {
        case 0:  cond_x = CURRENT_STATE.FLAG_Z; break;                 // EQ
        case 1:  cond_x = !CURRENT_STATE.FLAG_Z; break;                // NE
        case 10: cond_x = !CURRENT_STATE.FLAG_N; break;                // GE
        case 11: cond_x = CURRENT_STATE.FLAG_N && !CURRENT_STATE.FLAG_Z; break; // LT
        case 12: cond_x = !CURRENT_STATE.FLAG_Z && !CURRENT_STATE.FLAG_N; break; // GT
        case 13: cond_x = CURRENT_STATE.FLAG_N; break;                 // LE
    }

    if (cond_x) {
        NEXT_STATE.PC += inst->immr - 4;
    }
}

void branch_conditional(instruction_t* inst) {
    conditional_branch(inst);
}

void conditional_branch_zero(instruction_t* inst) {
    if (NEXT_STATE.REGS[inst->Rd] == 0) {
        NEXT_STATE.PC += inst->immr - 4;
    }
}

void conditional_branch_nonzero(instruction_t* inst) {
    if (NEXT_STATE.REGS[inst->Rd] != 0) {
        NEXT_STATE.PC += inst->immr - 4;
    }
}

void compare_and_branch_zero(instruction_t* inst) {
    conditional_branch_zero(inst);
}

void compare_and_branch_nonzero(instruction_t* inst) {
    conditional_branch_nonzero(inst);
}

//
// ──────────────────────────────────────────────────────────────── MOVE ──────
//

void move_zero(instruction_t* inst) {
    NEXT_STATE.REGS[inst->Rd] = inst->immr;
    set_x31();
}
