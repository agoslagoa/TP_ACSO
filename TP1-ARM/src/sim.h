#ifndef SIM_H
#define SIM_H

#include <stdint.h>
#include "decoder.h"

#define PC_DIRECT_JUMP UINT64_MAX

// Etapas del ciclo de instrucción
uint32_t fetch_instruction();
Instruction decode_instruction(uint32_t raw_instruction);
uint64_t execute_instruction(const Instruction* inst);
void process_instruction();

#endif // SIM_H