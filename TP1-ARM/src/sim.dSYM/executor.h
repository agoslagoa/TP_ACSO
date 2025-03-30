#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdint.h>
#include "decoder.h"

// Ejecuta una instrucción previamente decodificada.
// Devuelve el desplazamiento del PC o PC_DIRECT_JUMP para saltos directos.
uint64_t execute(const Instruction* inst);

#endif // EXECUTOR_H