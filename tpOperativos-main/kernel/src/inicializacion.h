#ifndef INICIALIZACIONH
#define INICIALIZACIONH

#include "bibliotecas.h"

void inicializar_kernel( char *path_config );

// Loggers 

extern t_log* kernel_logger; 

// Semaforos

extern pthread_mutex_t mutex_queue_new;
extern pthread_mutex_t mutex_queue_ready_threads;
extern pthread_mutex_t mutex_list_blocked_threads;
extern pthread_mutex_t mutex_exit_threads;
extern pthread_mutex_t mutex_cola_de_prioridad;
extern pthread_mutex_t mutex_hilo_en_ejecucion;
extern pthread_mutex_t hilos_bloqueados_thread_join_mtx;
extern pthread_mutex_t proceso_en_ejecucion_mutex;
extern pthread_mutex_t creando_colas_por_prioridad;

extern sem_t hilo_interrumpido;
extern sem_t sem_CPU_libre_para_hilo;
extern sem_t espacio_para_proceso;
extern sem_t sem_en_syscall;
extern sem_t bloqueo_por_dump;
extern sem_t nuevos_hilos;
extern sem_t proceso_nuevo;
extern sem_t nuevos_hilos_para_cola;

// Conexiones

extern int fd_conexion_dispatch;
extern int fd_conexion_interrupt;

// Colas de hilos y procesos

extern t_queue* queue_new_processes; // Cola de procesos nuevos

extern t_queue*   queue_ready_threads; // Cola de hilos listos
extern t_list*   list_blocked_threads; // Cola de hilos bloqueados
extern t_queue*   queue_exit_threads; // Cola de hilos finalizados

void inicializar_kernel(char* path_config);

#endif /* INICIALIZACION_H */