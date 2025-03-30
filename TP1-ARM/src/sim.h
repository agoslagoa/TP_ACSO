/**
 * @file sim.h
 * @brief Instruction cycle stage declarations for the ARMv8 simulator.
 */

#ifndef SIM_H
#define SIM_H

#include <stdint.h>
#include "decoder.h"

/**
 * @brief Special marker indicating a direct PC jump (used for BR).
 */
#define PC_DIRECT_JUMP UINT64_MAX

/* ─────────────────────────────────────────────────────────────────────────────
 * FUNCTION DECLARATIONS
 * ───────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Fetches a 32-bit instruction from memory at the current PC.
 * 
 * @return Raw 32-bit instruction.
 */
uint32_t fetch_instruction();

/**
 * @brief Decodes a raw instruction into a structured format.
 * 
 * @param raw_instruction Raw instruction.
 * @return Decoded instruction structure.
 */
Instruction decode_instruction(uint32_t raw_instruction);

/**
 * @brief Executes a decoded instruction and returns the PC offset or jump signal.
 * 
 * @param inst Pointer to decoded instruction.
 * @return Offset to update PC, or PC_DIRECT_JUMP.
 */
uint64_t execute_instruction(const Instruction* inst);

/**
 * @brief Main function for simulating a single instruction cycle.
 *        Handles fetch, decode, execute, and PC update.
 */
void process_instruction();

#endif // SIM_H

// final version