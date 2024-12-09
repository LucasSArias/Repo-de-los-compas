#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include "bibliotecas.h"

// Estructura de configuracion del Kernel

typedef struct 
{
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* IP_CPU;
    char* PUERTO_CPU_DISPATCH;
    char* PUERTO_CPU_INTERRUPT;
    char* ALGORITMO_PLANIFICACION;
    int   QUANTUM;
}t_kernel_config;

extern t_kernel_config kernel_config;

typedef struct tcb tcb;

// Estructura de PCB

typedef struct 
{
    int pid;                          // Identificador del proceso
    tcb* hilo0;                       // Hilo 0 asociado al proceso
    t_list* tid_list;                 // Lista de Tid
    t_list* mutex;                    // Lista de mutexes
    int tam_memoria_pseudocodigo;     // TamaÃ±o de memoria del pseudocodigo
} pcb;

extern int pid_generator;

extern pcb* process_to_execute;

// Funciones de PCB

pcb* crear_pcb(int tam_memoria_pseudocodigo);
void liberar_pcb(pcb* proceso);

// Estructura de TCB

typedef struct tcb
{
    int tid;            // Identificador del hilo
    int prioridad;      // Prioridad del hilo
    char* pseudocodigo; // Path de pseudocodigo
    pcb* proceso;       // Proceso al que pertenece
    t_list* hilos_bloqueados; // Lista de hilos bloqueados por thread_join de este hilo
} tcb;

extern tcb* hilo_en_ejecucion;

// Funciones de TCB

tcb* crear_tcb(int tid, int prioridad, char* pseudocodigo);
tcb* buscar_tcb_por_tid_y_pid(int tid, int pid);
bool member_of_thread_queue(t_queue* queue, int tid);
void liberar_tcb(tcb* hilo);


// Estructura de Colas por Prioridad

typedef struct {
    t_queue* queue;
    int prioridad;
} priority_queue;

extern t_list *colas_prioridad;

// Estructura de Mutex

extern t_list* mutex_lista;

typedef struct{
    char* mutex_nombre;
    int disponible;
    tcb* hilo_duenio;
    t_queue* bloqueados;
} mutex_t;

// Estructura de Syscalls

typedef struct{
    char* pseudocodigo;
    int tam_memoria;
    int prioridad_hilo;
    int tid;
    int milisegundos;
    char* recurso_mutex;
} syscall_args;

typedef struct{
    syscall_t tipo;
    syscall_args argumentos;
} syscall_recibida;

// SEMAFOROS

void push_queue_new(void *elemento);
void push_queue_ready(void *elemento);
void list_add_blocked(void *elemento);
void push_queue_exit(void *elemento);

pcb* pop_queue_new();
tcb* pop_queue_ready();
void list_remove_blocked(tcb* hilo);
tcb* pop_queue_exit();

// Semaforos

void wait_sem(sem_t *sem);
void signal_sem(sem_t *sem);

// MUTEX

void wait_mutex(pthread_mutex_t *sem);
void signal_mutex(pthread_mutex_t *sem);
mutex_t* buscar_mutex_por_nombre(char* nombre);
#endif /* ESTRUCTURAS_H_ */