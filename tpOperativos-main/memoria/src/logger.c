#include <logger.h>

static const char* _decide_operacion(e_log_condition condition);

void iniciar_logger_memoria()
{
    memoria_logger = iniciar_logger("memoria.log", MEMORIA, memoria_config->log_level);
}

void log_obligatorio_conex_kernel(int fd)
{
    log_info(memoria_logger, "## Kernel Conectado - FD del socket: %d", fd);
}

void log_obligatorio_proceso(e_log_condition condition, int pid, int tamanio)
{
    log_info(memoria_logger, "## Proceso %s -  PID: %d - Tamaño: %d",
             _decide_operacion(condition), pid, tamanio);
}

void log_obligatorio_hilo(e_log_condition condition, int pid, int tid)
{
    log_info(memoria_logger, "## Hilo %s - (PID:TID) - (%d:%d)",
             _decide_operacion(condition), pid, tid);
}

void log_obligatorio_contexto(e_log_condition condition, int pid, int tid)
{
    log_info(memoria_logger, "## Contexto %s - (PID:TID) - (%d:%d)",
             _decide_operacion(condition), pid, tid);
}

void log_obligatorio_instruccion(int pid, int tid, char *instruccion)
{
    log_info(memoria_logger, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s",
             pid, tid, instruccion);
}

void log_obligatorio_memoria_usuario(e_log_condition condition, int pid, int tid, int direccion_fisica, int tamanio)
{
    log_info(memoria_logger, "## %s - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d",
             _decide_operacion(condition), pid, tid, direccion_fisica, tamanio);
}

void log_obligatorio_memory_dump(int pid, int tid)
{
    log_info(memoria_logger, "## Memory Dump solicitado - (PID:TID) - (%d:%d)", pid, tid);
}

static const char* _decide_operacion(e_log_condition condition)
{
    switch (condition)
    {
    case CREACION:
        return "Creacion";
    case DESTRUCCION:
        return "Destruccion";
    case SOLICITADO:
        return "Solicitado";
    case ACTUALIZADO:
        return "Actualizado";
    case ESCRITURA:
        return "Escritura";
    case LECTURA:
        return "Lectura";
    default:
        return "Desconocido";
    }
}