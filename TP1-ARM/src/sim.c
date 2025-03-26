#include "shell.h"
#include "decode_core.h"
#include "instruction.h"

/*
 * Fetches the instruction at the current PC, decodes it, and executes it.
 * If the instruction does not modify the PC (e.g., branch), PC is incremented by 4.
 * Ensures register X31 is always zero after execution.
 */
void process_instruction() {
    instruction_t inst = {0};  // Initialize empty instruction

    // Fetch instruction from memory
    inst.value = mem_read_32(CURRENT_STATE.PC);

    // Decode and execute if valid
    if (decode(&inst)) {
        execute(&inst);

        // If PC was not modified by the instruction, increment by 4
        if (NEXT_STATE.PC == CURRENT_STATE.PC) {
            NEXT_STATE.PC += 4;
        }
    }

    // Ensure X31 is always zero
    NEXT_STATE.REGS[31] = 0;
}
