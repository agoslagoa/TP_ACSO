#include "decode_core.h"
#include "instruction.h"
#include "opcodes.h"
#include "shell.h"
#include <stdint.h>
#include <stdio.h>

uint32_t create_mask(uint8_t length, uint8_t shift) {
    return ((1U << length) - 1) << shift;
}

uint32_t extract_bits(uint32_t instr, uint8_t length, uint8_t shift) {
    return (instr & create_mask(length, shift)) >> shift;
}

uint32_t get_field(instruction_t* inst, uint8_t length, uint8_t shift) {
    return extract_bits(inst->value, length, shift);
}

// ───────────────────────────── FORMATS DECODING ─────────────────────────────

void decode_load_store(instruction_t* inst) {
    inst->Rn = get_field(inst, 5, 5);
    inst->Rd = get_field(inst, 5, 0);
    int64_t val = get_field(inst, 9, 12);
    inst->immr = ((int64_t)(val << (64 - 9))) >> (64 - 9);
}

void decode_logic(instruction_t* inst) {
    inst->Rm = get_field(inst, 5, 16);
    inst->Rn = get_field(inst, 5, 5);
    inst->Rd = get_field(inst, 5, 0);
}

void decode_arith_reg(instruction_t* inst) {
    inst->Rm = get_field(inst, 5, 16);
    inst->Rn = get_field(inst, 5, 5);
    inst->Rd = get_field(inst, 5, 0);
}

void decode_arith_imm(instruction_t* inst) {
    inst->Rd = get_field(inst, 5, 0);
    inst->Rn = get_field(inst, 5, 5);
    inst->immr = get_field(inst, 12, 10);
    inst->shift = get_field(inst, 2, 22);
}

void decode_shift(instruction_t* inst) {
    inst->Rd = get_field(inst, 5, 0);
    inst->Rn = get_field(inst, 5, 5);
    inst->immr = get_field(inst, 6, 16);
    inst->imms = get_field(inst, 6, 10);
}

void decode_branch(instruction_t* inst) {
    int32_t offset = get_field(inst, 26, 0);
    inst->immr = ((int64_t)(offset << (64 - 26))) >> (64 - 26);
    inst->immr <<= 2;
}

void decode_branch_reg(instruction_t* inst) {
    inst->Rn = get_field(inst, 5, 5);
}

void decode_bcond(instruction_t* inst) {
    int32_t offset = get_field(inst, 19, 5);
    inst->immr = ((int64_t)(offset << (64 - 19))) >> (64 - 19);
    inst->immr <<= 2;
    inst->cond = get_field(inst, 4, 0);
}

void decode_movz(instruction_t* inst) {
    inst->Rd = get_field(inst, 5, 0);
    inst->immr = get_field(inst, 16, 5);
}

void decode_cbz_cbnz(instruction_t* inst) {
    inst->Rd = get_field(inst, 5, 0);
    int32_t offset = get_field(inst, 19, 5);
    inst->immr = ((int64_t)(offset << (64 - 19))) >> (64 - 19);
    inst->immr <<= 2;
}

// ───────────────────────────── DECODING LOGIC ─────────────────────────────

bool decode(instruction_t* inst) {

    uint32_t val = inst->value;

    if ((val & 0xFFFFFC00) == 0xD4400000) {
        printf("HLT instruction encountered. Halting simulation.\n");
        RUN_BIT = 0;
        return false;
    }

    // Group 1: Load/store (offset 21, len 11)
    inst->opcode = get_field(inst, 11, 21);
    for (int i = 0; i < 6; i++) {
        if (inst->opcode == OPCODE_LOAD_STORE[i]) {
            printf("[decode] Load/Store: opcode = %#x, index = %d\n", inst->opcode, i);
            decode_load_store(inst);
            inst->operation_func = OPCODE_LOAD_STORE_FUNCS[i];
            return true;
        }
    }

    // Group 2: Logical ops
    inst->opcode = get_field(inst, 11, 21);
    for (int i = 0; i < 3; i++) {
        if (inst->opcode == OPCODE_LOGIC[i]) {
            printf("[decode] Logic: opcode = %#x, index = %d\n", inst->opcode, i);
            decode_logic(inst);
            inst->operation_func = OPCODE_LOGIC_FUNCS[i];
            return true;
        }
    }

    // Group 3: Arithmetic reg
    inst->opcode = get_field(inst, 11, 21);
    for (int i = 0; i < 4; i++) {
        if (inst->opcode == OPCODE_ARITH_REG[i]) {
            printf("[decode] Arithmetic reg: opcode = %#x, index = %d\n", inst->opcode, i);
            decode_arith_reg(inst);
            inst->operation_func = OPCODE_ARITH_REG_FUNCS[i];
            return true;
        }
    }

    // Group 4: Arithmetic imm
    inst->opcode = get_field(inst, 10, 22);
    for (int i = 0; i < 3; i++) {
        if (inst->opcode == OPCODE_ARITH_IMM[i]) {
            printf("[decode] Arithmetic imm: opcode = %#x, index = %d\n", inst->opcode, i);
            decode_arith_imm(inst);
            inst->operation_func = OPCODE_ARITH_IMM_FUNCS[i];
            return true;
        }
    }

    // Group 5: Shift
    inst->opcode = get_field(inst, 10, 22);
    if (inst->opcode == OPCODE_SHIFT[0]) {
        printf("[decode] Shift: opcode = %#x\n", inst->opcode);
        decode_shift(inst);
        inst->operation_func = OPCODE_SHIFT_FUNCS[0];
        return true;
    }

    // Group 6: Branch
    inst->opcode = get_field(inst, 6, 26);
    if (inst->opcode == OPCODE_BRANCH[0]) {
        printf("[decode] Branch (unconditional): opcode = %#x\n", inst->opcode);
        decode_branch(inst);
        inst->operation_func = OPCODE_BRANCH_FUNCS[0];
        return true;
    }

    // Group 7: Branch reg
    inst->opcode = get_field(inst, 22, 0);
    if (inst->opcode == OPCODE_BRANCH_REG[0]) {
        printf("[decode] Branch register: opcode = %#x\n", inst->opcode);
        decode_branch_reg(inst);
        inst->operation_func = OPCODE_BRANCH_REG_FUNCS[0];
        return true;
    }

    // Group 8: b.cond
    inst->opcode = get_field(inst, 8, 24);
    if (inst->opcode == OPCODE_BRANCH_COND[0]) {
        printf("[decode] Conditional branch: opcode = %#x\n", inst->opcode);
        decode_bcond(inst);
        inst->operation_func = OPCODE_BRANCH_COND_FUNCS[0];
        return true;
    }

    // Group 9: MOVZ
    inst->opcode = get_field(inst, 11, 21);
    if (inst->opcode == OPCODE_MOVE_IMM[0]) {
        printf("[decode] Move immediate (MOVZ): opcode = %#x\n", inst->opcode);
        decode_movz(inst);
        inst->operation_func = OPCODE_MOVE_IMM_FUNCS[0];
        return true;
    }

    // Group 10: CBZ/CBNZ
    inst->opcode = get_field(inst, 8, 24);
    for (int i = 0; i < 2; i++) {
        if (inst->opcode == OPCODE_CMP_BRANCH[i]) {
            printf("[decode] Compare and branch: opcode = %#x, index = %d\n", inst->opcode, i);
            decode_cbz_cbnz(inst);
            inst->operation_func = OPCODE_CMP_BRANCH_FUNCS[i];
            return true;
        }
    }

    printf("Unknown instruction: 0x%08X\n", inst->value);
    RUN_BIT = 0;
    return false;
}


void execute(instruction_t* inst) {
    if (inst->operation_func) {
        inst->operation_func(inst);
    } else {
        printf("No function associated with instruction.\n");
    }
}
