/**
 * @file executor.c
 * @brief Implementation of the instruction executor for the ARMv8 simulator.
 */

#include "shell.h"
#include "executor.h"
#include "decoder.h"
#include <string.h>
#include <stdint.h>

#define PC_DIRECT_JUMP UINT64_MAX  ///< Special value to signal absolute PC jump (e.g., BR)

/**
 * @brief Updates the condition flags Z (zero) and N (negative) based on the result.
 * 
 * @param result The result of the operation to be evaluated.
 */
void set_flags(int64_t result) {
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result < 0);
}

/**
 * @brief Executes the given decoded instruction.
 * 
 * @param inst Pointer to the decoded instruction.
 * @return uint64_t - Relative PC offset, or PC_DIRECT_JUMP for absolute jumps, or 0 by default.
 */
uint64_t execute(const Instruction* inst) {
    uint64_t offset = 0;

    // Arithmetic and logic with flags
    if (strcmp(inst->name, "ADDS_IMM") == 0) {
        int64_t imm = (inst->shift == 1) ? (inst->imm << 12) : inst->imm;
        int64_t result = CURRENT_STATE.REGS[inst->Rn] + imm;
        NEXT_STATE.REGS[inst->Rd] = result;
        set_flags(result);
    } else if (strcmp(inst->name, "SUBS_IMM") == 0) {
        int64_t imm = (inst->shift == 1) ? (inst->imm << 12) : inst->imm;
        int64_t result = CURRENT_STATE.REGS[inst->Rn] - imm;
        NEXT_STATE.REGS[inst->Rd] = result;
        set_flags(result);
    } else if (strcmp(inst->name, "ADDS_EXT") == 0) {
        int64_t result = CURRENT_STATE.REGS[inst->Rn] + CURRENT_STATE.REGS[inst->Rm];
        NEXT_STATE.REGS[inst->Rd] = result;
        set_flags(result);
    } else if (strcmp(inst->name, "SUBS_EXT") == 0) {
        int64_t result = CURRENT_STATE.REGS[inst->Rn] - CURRENT_STATE.REGS[inst->Rm];
        NEXT_STATE.REGS[inst->Rd] = result;
        set_flags(result);
    } else if (strcmp(inst->name, "CMP") == 0) {
        int64_t result = CURRENT_STATE.REGS[inst->Rn] - CURRENT_STATE.REGS[inst->Rm];
        set_flags(result);
    } else if (strcmp(inst->name, "CMP_IMM") == 0) {
        int64_t imm = (inst->shift == 1) ? (inst->imm << 12) : inst->imm;
        int64_t result = CURRENT_STATE.REGS[inst->Rn] - imm;
        set_flags(result);
    }

    // Arithmetic and logic without flags
    else if (strcmp(inst->name, "MUL") == 0) {
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] * CURRENT_STATE.REGS[inst->Rm];
    } else if (strcmp(inst->name, "MOVZ") == 0) {
        NEXT_STATE.REGS[inst->Rd] = ((uint64_t)inst->imm) << inst->shift;
    } else if (strcmp(inst->name, "ADD") == 0) {
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] + CURRENT_STATE.REGS[inst->Rm];
    } else if (strcmp(inst->name, "ADDI") == 0) {
        int64_t imm = (inst->shift == 1) ? (inst->imm << 12) : inst->imm;
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] + imm;
    } else if (strcmp(inst->name, "SUB") == 0) {
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] - CURRENT_STATE.REGS[inst->Rm];
    } else if (strcmp(inst->name, "SUBI") == 0) {
        int64_t imm = (inst->shift == 1) ? (inst->imm << 12) : inst->imm;
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] - imm;
    } else if (strcmp(inst->name, "ANDS") == 0) {
        int64_t result = CURRENT_STATE.REGS[inst->Rn] & CURRENT_STATE.REGS[inst->Rm];
        NEXT_STATE.REGS[inst->Rd] = result;
        set_flags(result);
    } else if (strcmp(inst->name, "EOR") == 0) {
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] ^ CURRENT_STATE.REGS[inst->Rm];
    } else if (strcmp(inst->name, "ORR") == 0) {
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] | CURRENT_STATE.REGS[inst->Rm];
    } else if (strcmp(inst->name, "LSL") == 0) {
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] << (64 - inst->imm);
    } else if (strcmp(inst->name, "LSR") == 0) {
        NEXT_STATE.REGS[inst->Rd] = CURRENT_STATE.REGS[inst->Rn] >> (inst->imm - 64);
    }

    // Branches and control flow
    else if (strcmp(inst->name, "B") == 0) {
        offset = inst->imm;
    } else if (strcmp(inst->name, "BR") == 0) {
        return PC_DIRECT_JUMP;
    } else if (strcmp(inst->name, "B.cond") == 0) {
        int take_branch = 0;
        switch (inst->cond) {
            case 0:  take_branch = (CURRENT_STATE.FLAG_Z == 1); break;  // EQ
            case 1:  take_branch = (CURRENT_STATE.FLAG_Z == 0); break;  // NE
            case 10: take_branch = (CURRENT_STATE.FLAG_N == 0); break;  // GE
            case 11: take_branch = (CURRENT_STATE.FLAG_N == 1); break;  // LT
            case 12: take_branch = (CURRENT_STATE.FLAG_Z == 0 && CURRENT_STATE.FLAG_N == 0); break; // GT
            case 13: take_branch = !(CURRENT_STATE.FLAG_Z == 0 && CURRENT_STATE.FLAG_N == 0); break; // LE
        }
        if (take_branch) return inst->imm;
    } else if (strcmp(inst->name, "CBZ") == 0) {
        if (CURRENT_STATE.REGS[inst->Rn] == 0) return inst->imm;
    } else if (strcmp(inst->name, "CBNZ") == 0) {
        if (CURRENT_STATE.REGS[inst->Rn] != 0) return inst->imm;
    } else if (strcmp(inst->name, "HLT") == 0) {
        RUN_BIT = 0;
    }

    // Memory instructions
    else if (strcmp(inst->name, "LDUR") == 0) {
        uint64_t addr = CURRENT_STATE.REGS[inst->Rn] + inst->imm;
        uint64_t low = mem_read_32(addr);
        uint64_t high = mem_read_32(addr + 4);
        NEXT_STATE.REGS[inst->Rt] = (high << 32) | low;
    } else if (strcmp(inst->name, "LDURB") == 0) {
        uint64_t addr = CURRENT_STATE.REGS[inst->Rn] + inst->imm;
        NEXT_STATE.REGS[inst->Rt] = mem_read_32(addr) & 0xFF;
    } else if (strcmp(inst->name, "LDURH") == 0) {
        uint64_t addr = CURRENT_STATE.REGS[inst->Rn] + inst->imm;
        NEXT_STATE.REGS[inst->Rt] = mem_read_32(addr) & 0xFFFF;
    } else if (strcmp(inst->name, "STUR") == 0) {
        uint64_t addr = CURRENT_STATE.REGS[inst->Rn] + inst->imm;
        uint64_t val = CURRENT_STATE.REGS[inst->Rt];
        mem_write_32(addr, val & 0xFFFFFFFF);
        mem_write_32(addr + 4, (val >> 32) & 0xFFFFFFFF);
    } else if (strcmp(inst->name, "STURB") == 0) {
        uint64_t addr = CURRENT_STATE.REGS[inst->Rn] + inst->imm;
        uint32_t word = mem_read_32(addr & ~0x3);
        uint8_t byte = CURRENT_STATE.REGS[inst->Rt] & 0xFF;
        int offset = addr & 0x3;
        word &= ~(0xFF << (offset * 8));
        word |= (byte << (offset * 8));
        mem_write_32(addr & ~0x3, word);
    } else if (strcmp(inst->name, "STURH") == 0) {
        uint64_t addr = CURRENT_STATE.REGS[inst->Rn] + inst->imm;
        uint32_t word = mem_read_32(addr & ~0x3);
        uint16_t half = CURRENT_STATE.REGS[inst->Rt] & 0xFFFF;
        int offset = addr & 0x3;
        word &= ~(0xFFFF << (offset * 8));
        word |= (half << (offset * 8));
        mem_write_32(addr & ~0x3, word);
    }

    return offset;
}
