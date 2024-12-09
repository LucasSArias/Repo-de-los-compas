#include "cpu_dispatch.h"

void atender_cpu_dispatch()
{ 
    // Limpio antes de recibir
    hilo_ejecutar = NULL;
    contexto_ejecucion = NULL;

    recibir_hilo_a_ejecutar_kernel();

    if(hilo_ejecutar == NULL){
        log_error(cpu_logger, "No se pudo recibir el hilo a ejecutar");
        return;
    }
    log_debug(cpu_logger, "Se recibio el hilo a ejecutar. PID: %d - TID: %d", hilo_ejecutar->pid, hilo_ejecutar->tid);
    
    log_info(cpu_logger, "## TID: %d - Solicito Contexto Ejecución", hilo_ejecutar->tid);
    recibir_contexto_ejecucion_memoria();

    if(contexto_ejecucion == NULL){
        log_error(cpu_logger, "No se pudo recibir el contexto de ejecucion");
        exit(EXIT_FAILURE);
    }
    
    log_debug(cpu_logger, "Iniciando ejecucion del hilo");
    ejecutar_hilo(hilo_ejecutar, contexto_ejecucion);
}

void liberar_ejecucion()
{
    liberar_hilo_ejecutar();
    liberar_contexto_ejecucion();
}

void liberar_hilo_ejecutar()
{
    free(hilo_ejecutar);    
}

void liberar_contexto_ejecucion()
{
    free(contexto_ejecucion);
}
