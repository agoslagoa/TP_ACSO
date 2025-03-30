#include "decode_core.h"
#include "opcodes.h"
#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//
// ──────────────────────────────────────────────────────── UTILITY MASK FUNCTIONS ─────
//

uint32_t generate_mask(uint8_t len, uint8_t move_left) {
    return ((1U << len) - 1) << move_left;
}

uint32_t apply_mask(instruction_t* inst, uint32_t mask, uint8_t move_right) {
    return (inst->value & mask) >> move_right;
}

uint32_t get_values(instruction_t* inst, uint8_t move, uint16_t amount_ones) {
    return apply_mask(inst, generate_mask(amount_ones, move), move);
}

//
// ──────────────────────────────────────────────────────────── FIELD EXTRACTION ─────
//

void extract_base_register(instruction_t* inst) {
    inst->Rn = get_values(inst, 5, 5);



}

void extract_destination_register(instruction_t* inst) {
    inst->Rd = get_values(inst, 0, 5);


}

void extract_second_register(instruction_t* inst) {
    inst->Rm = get_values(inst, 16, 5);


}

void extract_signed_immediate(instruction_t* inst, uint8_t move_left, uint8_t len) {
    int64_t value = get_values(inst, move_left, len);
    uint8_t shift = 64 - len;
    value = value << shift;
    inst->immr = value >> shift;
}

void extract_unsigned_immediate(instruction_t* inst, uint8_t move_left, uint8_t len) {
    inst->immr = get_values(inst, move_left, len);



}

void extract_shift_amount(instruction_t* inst) {
    inst->shift = get_values(inst, 22, 2);


}

void apply_branch_shift(instruction_t* inst) {
    inst->immr <<= 2;
}

//
// ────────────────────────────────────────────────────────────── INSTRUCTION FORMATS ─────
//

void format_load_store(instruction_t* inst) {
    extract_base_register(inst);
    extract_destination_register(inst);
    extract_signed_immediate(inst, 12, 9);
}

void format_three_registers(instruction_t* inst) {
    extract_second_register(inst);
    extract_base_register(inst);
    extract_destination_register(inst);
}

void format_immediate_arithmetic(instruction_t* inst) {
    extract_destination_register(inst);
    extract_base_register(inst);
    extract_unsigned_immediate(inst, 10, 12);
    extract_shift_amount(inst);
}

void format_shift(instruction_t* inst) {
    extract_destination_register(inst);
    extract_base_register(inst);
    extract_unsigned_immediate(inst, 16, 6);
    inst->imms = get_values(inst, 10, 6);
}

void format_unconditional_branch(instruction_t* inst) {
    extract_signed_immediate(inst, 0, 26);
    apply_branch_shift(inst);
}

void format_branch_register(instruction_t* inst) {
    extract_base_register(inst);
}

void format_conditional_branch(instruction_t* inst) {
    extract_signed_immediate(inst, 5, 19);
    apply_branch_shift(inst);
    inst->cond = get_values(inst, 0, 4);
}

void format_move_immediate(instruction_t* inst) {
    extract_destination_register(inst);
    extract_unsigned_immediate(inst, 5, 16);
}







void format_compare_and_branch(instruction_t* inst) {
    extract_destination_register(inst);
    extract_signed_immediate(inst, 5, 19);
    apply_branch_shift(inst);
}






//
// ────────────────────────────────────────────────────────────── SETUP & EXECUTION ─────
//

void set_parameters(instruction_t* inst, uint8_t opcode_type) {
    switch(opcode_type) {
        case 1:  format_load_store(inst);         break;
        case 2:  format_three_registers(inst);    break;
        case 3:  format_three_registers(inst);    break;
        case 4:  format_immediate_arithmetic(inst); break;
        case 5:  format_shift(inst);              break;
        case 6:  format_unconditional_branch(inst); break;
        case 7:  format_branch_register(inst);    break;
        case 8:  format_conditional_branch(inst); break;
        case 9:  format_move_immediate(inst);     break;
        case 10: format_compare_and_branch(inst); break;
    }
}

bool set_instruction(instruction_t* inst, int32_t possible_opcodes[], uint8_t amount_op, FunctionPtr* type_functions, uint8_t type) {
    for (uint8_t i = 0; i < amount_op; i++) {
        if (inst->opcode == possible_opcodes[i]) {
            set_parameters(inst, type);
            inst->operation_func = type_functions[i];


            return true;
        }
    }

    return false;
}