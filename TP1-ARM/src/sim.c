#include "shell.h"
#include "decoder.h"
#include "executor.h"
#include <stdio.h>
#include <stdint.h>

#define PC_DIRECT_JUMP UINT64_MAX

// Fetches the raw 32-bit instruction from memory
uint32_t fetch_instruction() {
    return mem_read_32(CURRENT_STATE.PC);
}

// Decodes a raw instruction into a structured format
Instruction decode_instruction(uint32_t raw_instruction) {
    return decode(raw_instruction);
}

// Executes the instruction and returns the PC offset or target address
uint64_t execute_instruction(const Instruction* inst) {
    return execute(inst);
}

// Main function called by the shell for each cycle
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

    NEXT_STATE.REGS[31] = 0; // Ensure XZR is always zero
}
