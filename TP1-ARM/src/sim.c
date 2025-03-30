/**
 * @file sim.c
 * @brief ARMv8 simulation core. Fetches, decodes, and executes instructions.
 */

#include "shell.h"
#include "decoder.h"
#include "executor.h"
#include <stdio.h>
#include <stdint.h>

#define PC_DIRECT_JUMP UINT64_MAX  ///< Special marker for absolute jumps (e.g., BR)

/* ─────────────────────────────────────────────────────────────────────────────
 * FETCH STAGE
 * ───────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Fetches a 32-bit instruction from memory at the current PC.
 * 
 * @return uint32_t Raw instruction.
 */
uint32_t fetch_instruction() {
    return mem_read_32(CURRENT_STATE.PC);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * DECODE STAGE
 * ───────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Decodes a raw instruction using the decoder module.
 * 
 * @param raw_instruction Raw 32-bit instruction.
 * @return Instruction Struct representing the decoded instruction.
 */
Instruction decode_instruction(uint32_t raw_instruction) {
    return decode(raw_instruction);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * EXECUTE STAGE
 * ───────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Executes a decoded instruction and returns the PC update strategy.
 * 
 * @param inst Pointer to decoded instruction.
 * @return uint64_t Offset or special jump value.
 */
uint64_t execute_instruction(const Instruction* inst) {
    return execute(inst);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * PROCESS CYCLE
 * ───────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Main function to process a single CPU cycle: fetch, decode, execute.
 *        Handles PC updates and ensures XZR register remains zero.
 */
void process_instruction() {
    uint32_t raw = fetch_instruction();
    Instruction inst = decode_instruction(raw);

    if (!inst.valid) {
        printf("Unknown instruction at PC: 0x%lx\n", CURRENT_STATE.PC);
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
        return;
    }

    uint64_t offset = execute_instruction(&inst);

    if (offset == PC_DIRECT_JUMP) {
        NEXT_STATE.PC = inst.target_address;
    } else if (offset == 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    }

    // Ensure register XZR (register 31) is always zero.
    NEXT_STATE.REGS[31] = 0;
}

// final version