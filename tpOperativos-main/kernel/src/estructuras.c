#include "bibliotecas.h"
#include "estructuras.h"
#include "inicializacion.h"

t_kernel_config kernel_config;
int pid_generator = 0;
pcb *process_to_execute;
tcb *hilo_en_ejecucion;
t_list *colas_prioridad;

/* PCB */

// Creacion de un PCB
pcb *crear_pcb(int tam_memoria_pseudocodigo)
{
    pcb *nuevo_pcb = (pcb *)malloc(sizeof(pcb)); // Se asigna su espacio de memoria.
    if (!nuevo_pcb)
    {
        log_error(kernel_logger, "Error al crear PCB");
        exit(EXIT_FAILURE);
    }

    nuevo_pcb->pid = pid_generator;      // PID autoincremental
    nuevo_pcb->tid_list = list_create(); // Se inicia la lista de TID
    nuevo_pcb->mutex = list_create();    // Se inicia la lista de mutexes

    nuevo_pcb->tam_memoria_pseudocodigo = tam_memoria_pseudocodigo; // Se asigna el tamaño de memoria del pseudocodigo

    pid_generator++;

    return nuevo_pcb;
}

// Destruccion de un PCB
void liberar_pcb(pcb *proceso)
{
    list_destroy_and_destroy_elements(proceso->tid_list, free);
    list_destroy(proceso->mutex);
    // list_clean_and_destroy_elements(proceso->mutex, free);
    
    free(proceso);
}

/* TCB */

// Creacion de un TCB

tcb *crear_tcb(int tid, int prioridad, char *pseudocodigo)
{
    tcb *nuevo_tcb = (tcb *)malloc(sizeof(tcb)); // Se asigna su espacio de memoria.
    if (!nuevo_tcb)
    {
        log_error(kernel_logger, "Error al crear TCB");
        exit(EXIT_FAILURE);
    }

    nuevo_tcb->tid = tid;                                     // Se asigna el TID
    nuevo_tcb->prioridad = prioridad;                         // Se asigna la prioridad
    nuevo_tcb->pseudocodigo = string_duplicate(pseudocodigo); // Se asigna el path de pseudocodigo
    nuevo_tcb->proceso = process_to_execute; // Se asigna el proceso al que pertenece
    nuevo_tcb->hilos_bloqueados = list_create();
    return nuevo_tcb;
}

void liberar_tcb(tcb* hilo)
{
    free(hilo->pseudocodigo);
    list_destroy(hilo->hilos_bloqueados);
    free(hilo);
}

// Busqueda de un TCB por TID
tcb *find_by_tid_y_pid_match(t_list *lista, int tid_deseado, int pid_deseado)
{
    bool _tid_y_pid_match(void *ptr)
    {
        tcb *hilo = (tcb *)ptr;
        return (hilo->tid == tid_deseado && hilo->proceso->pid == pid_deseado);
    }
    return list_find(lista, _tid_y_pid_match);
}

tcb *buscar_tcb_por_tid_y_pid(int tid, int pid) 
{
    tcb *hilo = NULL;
    t_list *lista_auxiliar = list_create();
    if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0 || strcmp(kernel_config.ALGORITMO_PLANIFICACION, "CMN") == 0)
    {   
        wait_mutex(&mutex_cola_de_prioridad);
        for (int i = 0; i < list_size(colas_prioridad); i++)
        {
            priority_queue *cola = list_get(colas_prioridad, i);
            list_add_all(lista_auxiliar, cola->queue->elements);
        }
        hilo = find_by_tid_y_pid_match(lista_auxiliar, tid, pid);
        signal_mutex(&mutex_cola_de_prioridad);
        list_destroy(lista_auxiliar);
        return hilo;
    }
    else
    {
        wait_mutex(&mutex_queue_ready_threads);
        wait_mutex(&mutex_list_blocked_threads);
        wait_mutex(&mutex_exit_threads);
        list_add_all(lista_auxiliar, queue_ready_threads->elements);
        list_add_all(lista_auxiliar, list_blocked_threads);
        list_add_all(lista_auxiliar, queue_exit_threads->elements);
        hilo = find_by_tid_y_pid_match(lista_auxiliar, tid, pid);
        signal_mutex(&mutex_queue_ready_threads);
        signal_mutex(&mutex_list_blocked_threads);
        signal_mutex(&mutex_exit_threads);
        list_destroy(lista_auxiliar);
        return hilo;
    }
}

bool member_of_thread_queue(t_queue *queue, int tid)
{
    tcb *hilo = NULL;

    for (int i = 0; i < queue_size(queue); i++)
    {
        hilo = queue_peek(queue);
        if (hilo->tid == tid)
        {
            return true;
        }
    }
    return false;
}

/* SEMAFOROS */

// Push en cola de procesos nuevos y colas de hilos listos, bloqueados y finalizados

void push_queue_new(void *elemento)
{
    wait_mutex(&mutex_queue_new);
    queue_push(queue_new_processes, elemento);
    signal_mutex(&mutex_queue_new);
}

void push_queue_ready(void *elemento)
{
    wait_mutex(&mutex_queue_ready_threads);
    queue_push(queue_ready_threads, elemento);
    signal_mutex(&mutex_queue_ready_threads);
    if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0 || strcmp(kernel_config.ALGORITMO_PLANIFICACION, "CMN") == 0)
    {
        signal_sem(&nuevos_hilos_para_cola);
    }
    else if(strcmp(kernel_config.ALGORITMO_PLANIFICACION, "FIFO") == 0)
    {
        signal_sem(&nuevos_hilos);
    }
}

void list_add_blocked(void *elemento)
{
    wait_mutex(&mutex_list_blocked_threads);
    list_add(list_blocked_threads, elemento);
    signal_mutex(&mutex_list_blocked_threads);
}

void push_queue_exit(void *elemento)
{
    wait_mutex(&mutex_exit_threads);
    queue_push(queue_exit_threads, elemento);
    signal_mutex(&mutex_exit_threads);
}

// Pop en cola de procesos nuevos y colas de hilos listos, bloqueados y finalizados

pcb *pop_queue_new()
{
    wait_mutex(&mutex_queue_new);
    pcb *proceso = queue_pop(queue_new_processes);
    signal_mutex(&mutex_queue_new);
    return proceso;
}

tcb *pop_queue_ready()
{
    wait_mutex(&mutex_queue_ready_threads);
    tcb *hilo = queue_pop(queue_ready_threads);
    signal_mutex(&mutex_queue_ready_threads);
    return hilo;
}

void list_remove_blocked(tcb *hilo)
{
    wait_mutex(&mutex_list_blocked_threads);
    list_remove_element(list_blocked_threads, hilo);
    signal_mutex(&mutex_list_blocked_threads);
}

tcb *pop_queue_exit()
{
    wait_mutex(&mutex_exit_threads);
    tcb *hilo = queue_pop(queue_exit_threads);
    signal_mutex(&mutex_exit_threads);
    return hilo;
}

// Wait y Signal de un semaforo

void wait_mutex(pthread_mutex_t *sem)
{
    pthread_mutex_lock(sem);
}

void signal_mutex(pthread_mutex_t *sem)
{
    pthread_mutex_unlock(sem);
}

void wait_sem(sem_t *sem)
{
    sem_wait(sem);
}

void signal_sem(sem_t *sem)
{
    sem_post(sem);
}

mutex_t *buscar_mutex_por_nombre(char *nombre)
{
    bool mutex_llamado(void *ptr)
    {
        mutex_t *mutex = (mutex_t *)ptr;
        return strcmp(mutex->mutex_nombre, nombre) == 0;
    }
    return list_find(mutex_lista, mutex_llamado);
}