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

void add_registers(instruction_t* inst)        { arithmetic_operation(inst, op_add); }
void sub_registers(instruction_t* inst)        { arithmetic_operation(inst, op_sub); }
void adds_registers(instruction_t* inst)       { arithmetic_operation(inst, op_add); }
void subs_registers(instruction_t* inst)       { arithmetic_operation(inst, op_sub); }
void add_immediate(instruction_t* inst)        { arithmetic_operation(inst, op_add); }
void adds_immediate(instruction_t* inst)       { arithmetic_operation(inst, op_add); }
void subs_immediate(instruction_t* inst)       { arithmetic_operation(inst, op_sub); }






















void multiply(instruction_t* inst) {
    NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rn] * NEXT_STATE.REGS[inst->Rm];
    set_x31();
@@ -101,20 +83,18 @@
// ──────────────────────────────────────────────────────────────── SHIFT ──────
//


void shift(instruction_t* inst, uint32_t shift_type) {
    uint32_t shift = 64 - (uint32_t) inst->immr;

    if (shift_type == 0b111111) {
        NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rn] >> shift;
    } else {
        NEXT_STATE.REGS[inst->Rd] = NEXT_STATE.REGS[inst->Rn] << shift;
    }

    set_x31();
}


void logical_shift(instruction_t* inst) {
    shift(inst, inst->imms);
}
@@ -192,18 +172,18 @@
}

void conditional_branch(instruction_t* inst) {
    bool condition_met = false;

    switch (inst->cond) {
        case 0:  condition_met = CURRENT_STATE.FLAG_Z; break;                 // EQ
        case 1:  condition_met = !CURRENT_STATE.FLAG_Z; break;                // NE
        case 10: condition_met = !CURRENT_STATE.FLAG_N; break;                // GE
        case 11: condition_met = CURRENT_STATE.FLAG_N && !CURRENT_STATE.FLAG_Z; break; // LT
        case 12: condition_met = !CURRENT_STATE.FLAG_Z && !CURRENT_STATE.FLAG_N; break; // GT
        case 13: condition_met = CURRENT_STATE.FLAG_N; break;                 // LE
    }

    if (condition_met) {
        NEXT_STATE.PC += inst->immr - 4;
    }
}
@@ -239,4 +219,4 @@
void move_zero(instruction_t* inst) {
    NEXT_STATE.REGS[inst->Rd] = inst->immr;
    set_x31();
}