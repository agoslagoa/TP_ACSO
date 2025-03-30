/**
 * @file decoder.c
 * @brief Implements the instruction decoding logic for the ARMv8 simulator.
 */

#include "decoder.h"
#include <string.h>
#include <stdio.h>
#include "shell.h"

// ────────────────────────────────────────────────
// Pattern Structure Definition
// ────────────────────────────────────────────────

typedef struct {
    uint32_t mask;
    uint32_t opcode;
    const char* name;
    void (*extract_fields)(Instruction*, uint32_t);
} Pattern;

// ────────────────────────────────────────────────
// Field Extractors for Different Instructions
// ────────────────────────────────────────────────

/**
 * @brief Extract fields for ADDS (immediate variant)
 */
void extract_adds_imm(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
}

/**
 * @brief Extract fields for ADDS (extended register variant)
 */
void extract_adds_ext(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

/**
 * @brief Extract fields for CMP (register)
 */
void extract_cmp(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
    inst->Rd = 31;  // XZR
}

/**
 * @brief Extract fields for CMP (immediate)
 */
void extract_cmp_imm(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
    inst->Rd = 31;  // XZR
}

/**
 * @brief Extract fields for MUL
 */
void extract_mul(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

/**
 * @brief Extract fields for SUBS (extended register)
 */
void extract_subs_ext(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

/**
 * @brief Extract fields for MOVZ
 */
void extract_movz(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->imm = (raw >> 5) & 0xFFFF;
}

/**
 * @brief Extract fields for B (unconditional branch)
 */
void extract_b(Instruction* inst, uint32_t raw) {
    int32_t imm26 = raw & 0x3FFFFFF;
    if (imm26 & (1 << 25)) {
        imm26 |= 0xFC000000; // Sign extend
    }
    inst->imm = imm26;
    inst->target_address = CURRENT_STATE.PC + (imm26 << 2);
}

/**
 * @brief Extract fields for ADD (register)
 */
void extract_add_reg(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

/**
 * @brief Extract fields for ADDI (immediate)
 */
void extract_add_imm(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
}

/**
 * @brief Extract fields for SUB (register)
 */
void extract_sub_reg(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

/**
 * @brief Extract fields for SUBI (immediate)
 */
void extract_sub_imm(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
}

/**
 * @brief Extract fields for logical register operations (ANDS, EOR, ORR)
 */
void extract_logic_reg(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
    inst->shift = (raw >> 22) & 0x3;
}

/**
 * @brief Extract fields for shift instructions (LSL, LSR)
 */
void extract_shift(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 16) & 0x3F;    // uimm6
    inst->shift = (raw >> 10) & 0x3F;  // imms
}

/**
 * @brief Extract fields for BR (register branch)
 */
void extract_br(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    inst->target_address = CURRENT_STATE.REGS[inst->Rn];
}

/**
 * @brief Extract fields for conditional branches
 */
void extract_bcond(Instruction* inst, uint32_t raw) {
    int32_t imm19 = (raw >> 5) & 0x7FFFF;
    if (imm19 & (1 << 18)) imm19 |= 0xFFF80000;  // sign extend
    inst->imm = imm19 << 2;
    inst->cond = raw & 0xF;
    inst->target_address = CURRENT_STATE.PC + inst->imm;
}

/**
 * @brief Extract fields for CBZ and CBNZ
 */
void extract_cb(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    int32_t imm19 = (raw >> 10) & 0x7FFFF;
    if (imm19 & (1 << 18)) imm19 |= 0xFFF80000;
    inst->imm = imm19 << 2;
    inst->target_address = CURRENT_STATE.PC + inst->imm;
}

/**
 * @brief Extract fields for load/store instructions
 */
void extract_ldst(Instruction* inst, uint32_t raw) {
    inst->Rt = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    int32_t imm9 = (raw >> 12) & 0x1FF;
    if (imm9 & 0x100) {
        imm9 |= 0xFFFFFE00;  // sign-extend 9-bit
    }
    inst->imm = imm9;
}

// ────────────────────────────────────────────────
// Instruction Pattern Table
// ────────────────────────────────────────────────

#define ENTRY(mask, opcode, name, extractor) {mask, opcode, name, extractor}

Pattern patterns[] = {
    ENTRY(0xFFC00000, 0xB1000000, "ADDS_IMM", extract_adds_imm),
    ENTRY(0xFFE00000, 0x8B200000, "ADDS_EXT", extract_adds_ext),
    ENTRY(0xFFE00000, 0xEB000000, "SUBS_EXT", extract_subs_ext),
    ENTRY(0xFFE00000, 0xD2800000, "MOVZ",     extract_movz),
    ENTRY(0xFC000000, 0x14000000, "B",        extract_b),
    ENTRY(0xFFE00000, 0x8B000000, "ADD",      extract_add_reg),
    ENTRY(0xFFC00000, 0x91000000, "ADDI",     extract_add_imm),
    ENTRY(0xFFE00000, 0xCB000000, "SUB",      extract_sub_reg),
    ENTRY(0xFFC00000, 0xD1000000, "SUBI",     extract_sub_imm),
    ENTRY(0xFFE0FC00, 0x9B007C00, "MUL",      extract_mul),
    ENTRY(0xFFE0001F, 0xEB00001F, "CMP",      extract_cmp),
    ENTRY(0xFFC0001F, 0xF100001F, "CMP_IMM",  extract_cmp_imm),
    ENTRY(0xFFE0FC00, 0xF2000000, "ANDS",     extract_logic_reg),
    ENTRY(0xFFE0FC00, 0xD2000000, "EOR",      extract_logic_reg),
    ENTRY(0xFFE0FC00, 0xAA000000, "ORR",      extract_logic_reg),
    ENTRY(0xFFC00000, 0xD3400000, "LSL",      extract_shift),
    ENTRY(0xFFC00000, 0xD3400000, "LSR",      extract_shift),
    ENTRY(0xFFFFFC1F, 0xD61F0000, "BR",       extract_br),
    ENTRY(0xFF000010, 0x54000000, "B.cond",   extract_bcond),
    ENTRY(0x7F000000, 0x34000000, "CBZ",      extract_cb),
    ENTRY(0x7F000000, 0x35000000, "CBNZ",     extract_cb),
    ENTRY(0xFFC00000, 0xF8400000, "LDUR",     extract_ldst),
    ENTRY(0xFFC00000, 0x38400000, "LDURB",    extract_ldst),
    ENTRY(0xFFC00000, 0x78400000, "LDURH",    extract_ldst),
    ENTRY(0xFFC00000, 0xF8000000, "STUR",     extract_ldst),
    ENTRY(0xFFC00000, 0x38000000, "STURB",    extract_ldst),
    ENTRY(0xFFC00000, 0x78000000, "STURH",    extract_ldst),
};

const int NUM_PATTERNS = sizeof(patterns) / sizeof(Pattern);

// ────────────────────────────────────────────────
// Decoder Main Function
// ────────────────────────────────────────────────

/**
 * @brief Decodes a 32-bit instruction and returns the corresponding structure.
 */
Instruction decode(uint32_t raw) {
    Instruction inst = {0};
    inst.opcode = raw;
    inst.valid = false;

    for (int i = 0; i < NUM_PATTERNS; i++) {
        uint32_t masked = raw & patterns[i].mask;
        if (masked == patterns[i].opcode) {
            strncpy(inst.name, patterns[i].name, sizeof(inst.name) - 1);
            inst.valid = true;
            if (patterns[i].extract_fields) {
                patterns[i].extract_fields(&inst, raw);
            }
            break;
        }
    }

    return inst;
}
