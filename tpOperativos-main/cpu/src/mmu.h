#ifndef CPU_MMU_H_
#define CPU_MMU_H_

#include "bibliotecas.h"

#include "execute.h"

uint32_t obtener_direccion_logica(e_registro registro_direccion);
uint32_t traducir_direccion_logica(uint32_t direccion_logica);

#endif /* CPU_MMU_H_ */  