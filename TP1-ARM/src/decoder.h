#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <stdbool.h>

// Representa una instrucción decodificada
typedef struct {
    char name[16];          // Nombre simbólico de la instrucción ("ADD", "SUBS", etc.)
    uint32_t opcode;        // Valor de opcode crudo

    // Registros
    uint8_t Rd;
    uint8_t Rn;
    uint8_t Rm;
    uint8_t Rt;             // ← Agregado para instrucciones de memoria
    uint8_t cond;

    // Inmediatos y shifts
    int32_t imm;
    uint32_t shift;
    uint32_t uimm6;
    uint32_t imms;

    // Control
    bool valid;             // Si la instrucción fue reconocida
    uint64_t target_address; // Usado para saltos tipo BR, B.cond, CBZ, etc.

} Instruction;

// Función principal del decodificador
Instruction decode(uint32_t raw);

#endif // DECODER_H