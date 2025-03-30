#include "decoder.h"
#include <string.h>
#include <stdio.h>
#include "shell.h"

// Estructura auxiliar para patrones
typedef struct {
    uint32_t mask;
    uint32_t opcode;
    const char* name;
    void (*extract_fields)(Instruction*, uint32_t);
} Pattern;

// ─────────────────────────────────────────────── EXTRACTORES DE CAMPOS ─────

void extract_adds_imm(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
}

void extract_adds_ext(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

void extract_cmp(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
    inst->Rd = 31;  // XZR
}

void extract_cmp_imm(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
    inst->Rd = 31;  // XZR
}

void extract_mul(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}


void extract_subs_ext(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

void extract_movz(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->imm = (raw >> 5) & 0xFFFF;
}

void extract_b(Instruction* inst, uint32_t raw) {
    int32_t imm26 = raw & 0x3FFFFFF;
    if (imm26 & (1 << 25)) {
        imm26 |= 0xFC000000; // Sign extend
    }
    inst->imm = imm26;
    inst->target_address = CURRENT_STATE.PC + (imm26 << 2);
}

void extract_add_reg(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

void extract_add_imm(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
}

void extract_sub_reg(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
}

void extract_sub_imm(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 10) & 0xFFF;
    inst->shift = (raw >> 22) & 0x3;
}

void extract_logic_reg(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->Rm = (raw >> 16) & 0x1F;
    inst->shift = (raw >> 22) & 0x3;
}

void extract_shift(Instruction* inst, uint32_t raw) {
    inst->Rd = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    inst->imm = (raw >> 16) & 0x3F;    // uimm6
    inst->shift = (raw >> 10) & 0x3F;  // imms
}

void extract_br(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    inst->target_address = CURRENT_STATE.REGS[inst->Rn];
}

void extract_bcond(Instruction* inst, uint32_t raw) {
    int32_t imm19 = (raw >> 5) & 0x7FFFF;
    if (imm19 & (1 << 18)) imm19 |= 0xFFF80000;  // sign extend
    inst->imm = imm19 << 2;
    inst->cond = raw & 0xF;
    inst->target_address = CURRENT_STATE.PC + inst->imm;
}

void extract_cb(Instruction* inst, uint32_t raw) {
    inst->Rn = (raw >> 5) & 0x1F;
    int32_t imm19 = (raw >> 5 + 5) & 0x7FFFF;
    if (imm19 & (1 << 18)) imm19 |= 0xFFF80000;
    inst->imm = imm19 << 2;
    inst->target_address = CURRENT_STATE.PC + inst->imm;
}

void extract_ldst(Instruction* inst, uint32_t raw) {
    inst->Rt = raw & 0x1F;
    inst->Rn = (raw >> 5) & 0x1F;
    int32_t imm9 = (raw >> 12) & 0x1FF;
    if (imm9 & 0x100) {
        imm9 |= 0xFFFFFE00;  // sign-extend 9-bit
    }
    inst->imm = imm9;
}


// ──────────────────────────────────────────────── TABLA DE PATRONES ─────

#define ENTRY(mask, opcode, name, extractor) {mask, opcode, name, extractor}

Pattern patterns[] = {
    ENTRY(0xFFC00000, 0x91000000, "ADDS_IMM", extract_adds_imm),
    ENTRY(0xFFE00000, 0xEB000000, "SUBS_EXT", extract_subs_ext),
    ENTRY(0xFFE00000, 0xD2800000, "MOVZ",     extract_movz),
    ENTRY(0xFC000000, 0x14000000, "B",        extract_b),
    ENTRY(0xFFE00000, 0x8B000000, "ADD",    extract_add_reg),
    ENTRY(0xFFC00000, 0x91000000, "ADDI",   extract_add_imm),
    ENTRY(0xFFE00000, 0xCB000000, "SUB",    extract_sub_reg),
    ENTRY(0xFFC00000, 0xD1000000, "SUBI",   extract_sub_imm),
    ENTRY(0xFFE0FC00, 0x9B007C00, "MUL", extract_mul),               // 10011011000
    ENTRY(0xFFE0001F, 0xEB00001F, "CMP", extract_cmp),               // CMP es SUBS con Rd = XZR
    ENTRY(0xFFC0001F, 0xF100001F, "CMP_IMM", extract_cmp_imm),       // CMP inmediato con Rd = XZR
    ENTRY(0xFFE0FC00, 0xF2000000, "ANDS", extract_logic_reg), // 11101010000
    ENTRY(0xFFE0FC00, 0xD2000000, "EOR",  extract_logic_reg), // 11001010000
    ENTRY(0xFFE0FC00, 0xAA000000, "ORR",  extract_logic_reg), // 10101010000
    ENTRY(0xFFC00000, 0xD3400000, "LSL",  extract_shift),     // 1101001101 (LSL, LSR comparten opcode)
    ENTRY(0xFFC00000, 0xD3400000, "LSR",  extract_shift),
    ENTRY(0xFFFFFC1F, 0xD61F0000, "BR",     extract_br),      // opcode: 1101011000011111000000
    ENTRY(0xFF000010, 0x54000000, "B.cond", extract_bcond),   // opcode: 01010100
    ENTRY(0x7F000000, 0x34000000, "CBZ",    extract_cb),      // opcode: 10110100
    ENTRY(0x7F000000, 0x35000000, "CBNZ",   extract_cb),      // opcode: 10110101
    ENTRY(0xFFC00000, 0xF8400000, "LDUR",   extract_ldst),   // 11111000010
    ENTRY(0xFFC00000, 0x38400000, "LDURB",  extract_ldst),   // 00111000010
    ENTRY(0xFFC00000, 0x78400000, "LDURH",  extract_ldst),   // 01111000010
    ENTRY(0xFFC00000, 0xF8000000, "STUR",   extract_ldst),   // 11111000000
    ENTRY(0xFFC00000, 0x38000000, "STURB",  extract_ldst),   // 00111000000
    ENTRY(0xFFC00000, 0x78000000, "STURH",  extract_ldst),   // 01111000000
    ENTRY(0xFFE00000, 0x8B200000, "ADDS_EXT", extract_adds_ext),
};

const int NUM_PATTERNS = sizeof(patterns) / sizeof(Pattern);

// ──────────────────────────────────────────────── DECODIFICADOR ─────

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
