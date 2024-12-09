#ifndef CPU_CONEXIONES_H_
#define CPU_CONEXIONES_H_

#include "bibliotecas.h"

#include "inicializacion.h"
#include "cpu_dispatch.h"

extern int conexion_dispatch;
extern int conexion_interrupt;
extern int conexion_memoria;

void iniciar_conexiones_cpu();
void iniciar_conexion_memoria();
void iniciar_servidor_cpu();
void iniciar_servidor_dispatch();
void iniciar_servidor_interrupt();
void cerrar_conexiones_cpu();

void recibir_hilo_a_ejecutar_kernel();
void recibir_contexto_ejecucion_memoria();
char *recibir_instruccion_a_ejecutar_memoria(uint32_t pid, uint32_t tid, uint32_t pc);
uint32_t recibir_valor_read_mem();

void pedir_read_a_memoria(uint32_t direccion_fisica);
void pedir_escribir_a_memoria(uint32_t valor, uint32_t direccion_fisica);

void enviar_contexto_hilo_a_memoria();
void enviar_tid_y_motivo_a_kernel(uint32_t hilo, op_code motivo);

#endif /* CPU_CONEXIONES_H_ */