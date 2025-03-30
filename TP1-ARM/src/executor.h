#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdint.h>
#include "decoder.h"

// Valor especial para indicar un salto absoluto (por ejemplo, BR)
#define PC_DIRECT_JUMP UINT64_MAX

// Ejecuta la instrucción decodificada y devuelve:
// - offset relativo del PC (ej: B, CBZ)
// - o PC_DIRECT_JUMP para saltos absolutos (ej: BR)
// - o 0 para avanzar a la siguiente instrucción
uint64_t execute(const Instruction* inst);

#endif // EXECUTOR_H
