/**
 * @file executor.h
 * @brief Interface for the ARMv8 instruction execution module.
 */

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdint.h>
#include "decoder.h"

/**
 * @brief Special value indicating an absolute jump in the program counter (e.g., BR instruction).
 */
#define PC_DIRECT_JUMP UINT64_MAX

/**
 * @brief Executes a decoded ARM instruction and determines how the PC should be updated.
 * 
 * @param inst Pointer to the decoded instruction.
 * @return uint64_t 
 *         - Relative offset to add to the PC (e.g., for B, CBZ),
 *         - PC_DIRECT_JUMP to jump to a specific address (e.g., for BR),
 *         - 0 to proceed to the next instruction sequentially.
 */
uint64_t execute(const Instruction* inst);

#endif // EXECUTOR_H
