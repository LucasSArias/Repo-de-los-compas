#ifndef MEMORIA_LOGGER_H_
#define MEMORIA_LOGGER_H_

#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <utils/constants.h>
#include <utils/utils.h>
#include <config.h>

typedef enum LogCondition
{   
    CREACION,
    DESTRUCCION,
    SOLICITADO,
    ACTUALIZADO,
    ESCRITURA,
    LECTURA,
} e_log_condition;

void iniciar_logger_memoria();
void log_obligatorio_conex_kernel(int fd);
void log_obligatorio_proceso(e_log_condition condition, int pid, int tamanio);
void log_obligatorio_hilo(e_log_condition condition, int pid, int tid);
void log_obligatorio_contexto(e_log_condition condition, int pid, int tid);
void log_obligatorio_instruccion(int pid, int tid, char *instruccion);
void log_obligatorio_memoria_usuario(e_log_condition condition, int pid, int tid, int direccion_fisica, int tamanio);
void log_obligatorio_memory_dump(int pid, int tid);

extern t_log* memoria_logger;

#endif /* MEMORIA_LOGGER_H_ */