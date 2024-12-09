#include "inicializacion.h"
#include "estructuras.h"
#include "finalizacion.h"

// Conexiones

void cerrar_conexion_cpu_dispatch(){
    close(fd_conexion_dispatch);
    log_debug(kernel_logger, "Conexion con CPU Dispatch cerrada de forma exitosa.");
}

void cerrar_conexion_cpu_interrupt(){
    close(fd_conexion_interrupt);
    log_debug(kernel_logger, "Conexion con CPU Interrupt cerrada de forma exitosa.");
}

// Colas

void terminar_colas(){
    queue_clean_and_destroy_elements(queue_new_processes, free);
    queue_clean_and_destroy_elements(queue_ready_threads, free);
    queue_clean_and_destroy_elements(queue_exit_threads, free);
}

void terminar_listas(){
    list_destroy_and_destroy_elements(list_blocked_threads, free);
}

// Semáforos y mutex

void terminar_mutex(){
    pthread_mutex_destroy(&mutex_queue_new);
    pthread_mutex_destroy(&mutex_queue_ready_threads);
    pthread_mutex_destroy(&mutex_list_blocked_threads);
    pthread_mutex_destroy(&mutex_exit_threads);
    pthread_mutex_destroy(&mutex_cola_de_prioridad);
    pthread_mutex_destroy(&mutex_hilo_en_ejecucion);
    pthread_mutex_destroy(&hilos_bloqueados_thread_join_mtx);
    pthread_mutex_destroy(&proceso_en_ejecucion_mutex);
}

void terminar_semaforos(){
    sem_destroy(&hilo_interrumpido);
    sem_destroy(&sem_CPU_libre_para_hilo);
    sem_destroy(&espacio_para_proceso);
}

// Finalización del  Kernel

void finalizar_kernel(){
    log_debug(kernel_logger, "Finalizando Kernel");
    cerrar_conexion_cpu_dispatch();
    cerrar_conexion_cpu_interrupt();
    terminar_colas();
    terminar_listas();
    free(kernel_config.IP_MEMORIA);
    free(kernel_config.PUERTO_MEMORIA);
    free(kernel_config.IP_CPU);
    free(kernel_config.PUERTO_CPU_DISPATCH);
    free(kernel_config.PUERTO_CPU_INTERRUPT);
    free(kernel_config.ALGORITMO_PLANIFICACION);
    terminar_semaforos();
    log_destroy(kernel_logger);
}