/**
 * @file decoder.h
 * @brief Header for the ARMv8 instruction decoder.
 */

#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @struct Instruction
 * @brief Represents a decoded ARMv8 instruction.
 */
typedef struct {
    char name[16];           ///< Symbolic name of the instruction (e.g. "ADD", "SUBS")
    uint32_t opcode;         ///< Raw 32-bit opcode

    // Register fields
    uint8_t Rd;              ///< Destination register
    uint8_t Rn;              ///< First operand register
    uint8_t Rm;              ///< Second operand register
    uint8_t Rt;              ///< Target register for memory operations
    uint8_t cond;            ///< Condition code for conditional branches

    // Immediate and shift fields
    int32_t imm;             ///< Signed immediate value
    uint32_t shift;          ///< Shift amount or type
    uint32_t uimm6;          ///< Unsigned 6-bit immediate (used in shifts)
    uint32_t imms;           ///< Immediate field for shift masks

    // Control and metadata
    bool valid;              ///< Indicates whether the instruction was recognized
    uint64_t target_address; ///< Calculated target address for branch/jump instructions

} Instruction;

/**
 * @brief Decodes a raw 32-bit instruction into a structured Instruction.
 *
 * @param raw The 32-bit instruction to decode.
 * @return Instruction Decoded instruction.
 */
Instruction decode(uint32_t raw);

#endif // DECODER_H

// final version