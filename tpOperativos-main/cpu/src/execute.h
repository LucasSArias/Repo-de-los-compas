#ifndef CPU_EXECUTE_H_
#define CPU_EXECUTE_H_

#include "bibliotecas.h"

#include <utils/constants.h>
#include "ejecucion_hilos.h"
#include "mmu.h"

void setear_registro(e_registro registro, uint32_t valor);
void leer_memoria(e_registro registro_datos, e_registro registro_direccion);
void escribir_memoria(e_registro registro_datos, e_registro registro_direccion);
void sumar_registros(e_registro registro_destino, e_registro registro_origen);
void restar_registros(e_registro registro_destino, e_registro registro_origen);
void loggear_registro(e_registro registro);
void execute_jnz(e_registro registro, uint32_t instruccion);
int obtener_valor_de_registro(e_registro registro);

#endif /* CPU_EXECUTE_H_ */