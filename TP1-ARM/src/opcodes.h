#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>
#include "decode_core.h"
#include "instructions_functions.h"

// ────── OPCODE ARRAYS ──────────────────────────────────────────────────────

extern uint32_t OPCODE_LOAD_STORE[];
extern uint32_t OPCODE_LOGIC[];
extern uint32_t OPCODE_ARITH_REG[];
extern uint32_t OPCODE_ARITH_IMM[];
extern uint32_t OPCODE_SHIFT[];
extern uint32_t OPCODE_BRANCH[];
extern uint32_t OPCODE_BRANCH_REG[];
extern uint32_t OPCODE_BRANCH_COND[];
extern uint32_t OPCODE_MOVE_IMM[];
extern uint32_t OPCODE_CMP_BRANCH[];

// ────── FUNCTION POINTER ARRAYS ────────────────────────────────────────────

extern FunctionPtr OPCODE_LOAD_STORE_FUNCS[];
extern FunctionPtr OPCODE_LOGIC_FUNCS[];
extern FunctionPtr OPCODE_ARITH_REG_FUNCS[];
extern FunctionPtr OPCODE_ARITH_IMM_FUNCS[];
extern FunctionPtr OPCODE_SHIFT_FUNCS[];
extern FunctionPtr OPCODE_BRANCH_FUNCS[];
extern FunctionPtr OPCODE_BRANCH_REG_FUNCS[];
extern FunctionPtr OPCODE_BRANCH_COND_FUNCS[];
extern FunctionPtr OPCODE_MOVE_IMM_FUNCS[];
extern FunctionPtr OPCODE_CMP_BRANCH_FUNCS[];

#endif // OPCODES_H
