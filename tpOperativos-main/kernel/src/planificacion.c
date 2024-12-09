#include "planificacion.h"
#include "estructuras.h"
#include "inicializacion.h"
#include "conexiones.h"
#include "syscalls.h"

pthread_mutex_t proceso_en_ejecucion_mutex;;

// Planificador de largo plazo

void *planificador_largo_plazo(void *arg)
{
    while (1)
    {   
        wait_sem(&proceso_nuevo);
        wait_sem(&espacio_para_proceso);
        wait_mutex(&mutex_queue_new);
        if (!queue_is_empty(queue_new_processes))
        {
            pcb *proceso = queue_peek(queue_new_processes); // Obtengo el primer proceso de la cola pero no lo elimino
            signal_mutex(&mutex_queue_new);
            if (espacio_para_proceso_en_memoria(proceso)) // Verifica si se pudo inicializar el proceso en la memoria
            {
                log_debug(kernel_logger, "## (%d) - Estado: READY", proceso->pid);
                pop_queue_new();                  // Elimino el proceso de la cola
                push_queue_ready(proceso->hilo0); // Pongo en ready el hilo 0 del proceso
                signal_sem(&espacio_para_proceso);
                signal_sem(&proceso_nuevo);
            }
            else
            {
                log_warning(kernel_logger, "Memoria insuficiente para el proceso PID: %d", proceso->pid); // Aun no saque el proceso de la cola, se queda esperando
                signal_sem(&proceso_nuevo);
            }
        } else {
            signal_mutex(&mutex_queue_new);
            signal_sem(&espacio_para_proceso);
        }
    }
}

void finalizacion_procesos(pcb* proceso)
{
    notificar_finalizacion_proceso_memoria_y_esperar_confirmacion(proceso);
    //log_debug(kernel_logger, "## (%d) - Ya avise a memoria", proceso->pid);
    terminar_hilos_de_proceso(proceso);
    log_debug(kernel_logger, "## (%d) - Estado: EXIT", proceso->pid);
    liberar_pcb(proceso);
    signal_sem(&espacio_para_proceso);
    signal_sem(&proceso_nuevo);
}

void terminar_hilos_de_proceso(pcb* proceso){
    for(int i = 0; i < list_size(proceso->tid_list); i++){
        int *tid = list_get(proceso->tid_list, i);
        tcb* hilo = buscar_tcb_por_tid_y_pid(*tid, proceso->pid);
        if(hilo != NULL){
            remover_hilo(hilo);
            push_queue_exit(hilo);
            liberar_tcb(hilo);
        }
    }
}

void remover_hilo(tcb* hilo)
{
    // Remover el hilo de la cola de hilos listos
    wait_mutex(&mutex_queue_ready_threads);
    for (int i = 0; i < queue_size(queue_ready_threads); i++)
    {
        tcb* hilo_actual = queue_pop(queue_ready_threads);
        if (hilo_actual->tid != hilo->tid)
        {
            queue_push(queue_ready_threads, hilo_actual);
        }
    }
    signal_mutex(&mutex_queue_ready_threads);

    // Remover el hilo de la lista de hilos bloqueados
    wait_mutex(&mutex_list_blocked_threads);
    for (int i = 0; i < list_size(list_blocked_threads); i++)
    {
        tcb* hilo_actual = list_get(list_blocked_threads, i);
        if (hilo_actual->tid == hilo->tid && hilo_actual->proceso->pid == hilo->proceso->pid)
        {
            list_remove(list_blocked_threads, i);
            break;
        }
    }
    signal_mutex(&mutex_list_blocked_threads);

    // Remover el hilo de las colas de prioridad
    if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0 || strcmp(kernel_config.ALGORITMO_PLANIFICACION, "CMN") == 0){   
        wait_mutex(&mutex_cola_de_prioridad);
        for (int i = 0; i < list_size(colas_prioridad); i++)
        {
            priority_queue* cola = list_get(colas_prioridad, i);
            for (int j = 0; j < queue_size(cola->queue); j++)
            {
                tcb* hilo_actual = queue_pop(cola->queue);
                if (hilo_actual->tid == hilo->tid && hilo_actual->proceso->pid == hilo->proceso->pid)
                {
                    continue;
                }
                queue_push(cola->queue, hilo_actual);
            }
        }
        signal_mutex(&mutex_cola_de_prioridad);
        return;
    }
}

// Planificador de corto plazo

/* ALGORITMOS DE PLANIFICACION */

// Planificador FIFO
void *planificar_fifo_hilos(void *args)
{
    while (1)
    {
        wait_sem(&nuevos_hilos);
        wait_sem(&sem_CPU_libre_para_hilo);
        wait_mutex(&mutex_queue_ready_threads);
        if (!queue_is_empty(queue_ready_threads))
        {
            signal_mutex(&mutex_queue_ready_threads);
            tcb *hilo = pop_queue_ready();
            poner_en_ejecucion(hilo);
            signal_sem(&nuevos_hilos);
        } else {
            signal_mutex(&mutex_queue_ready_threads);
            signal_sem(&sem_CPU_libre_para_hilo);
        }
    }
}

// Planificador por prioridades
void *planificar_prioridades_hilos(void *args)
{
    while (1)
    {
        wait_sem(&nuevos_hilos);
        wait_sem(&sem_CPU_libre_para_hilo);
        wait_mutex(&mutex_cola_de_prioridad);
        priority_queue *priority_queue = devolver_lista_maxima_prioridad();
        if (priority_queue == NULL)
        {
            signal_mutex(&mutex_cola_de_prioridad);
            signal_sem(&sem_CPU_libre_para_hilo);
        }
        else if (!queue_is_empty(priority_queue->queue))
        {
            tcb *hilo = queue_pop(priority_queue->queue);
            if (hilo == NULL)
            {
                log_error(kernel_logger, "Error al obtener hilo de la cola de prioridad");
            }
            signal_mutex(&mutex_cola_de_prioridad);
            poner_en_ejecucion(hilo);
            signal_sem(&nuevos_hilos);
        }
        else
        {
            list_remove_element(colas_prioridad, priority_queue);
            signal_mutex(&mutex_cola_de_prioridad);
            signal_sem(&sem_CPU_libre_para_hilo);
        }
    }
}

// Planificador de colas multinivel
void *planificar_cmn_hilos(void *args)
{ // A chequear
    while (1)
    {
        wait_sem(&nuevos_hilos);
        wait_sem(&sem_CPU_libre_para_hilo);
        wait_mutex(&mutex_cola_de_prioridad);
        priority_queue *priority_queue = devolver_lista_maxima_prioridad();
        if (priority_queue == NULL)
        {
            signal_mutex(&mutex_cola_de_prioridad);
            signal_sem(&sem_CPU_libre_para_hilo);
        }
        else if (!queue_is_empty(priority_queue->queue))
        {
            planificar_RR(priority_queue->queue);
            signal_sem(&nuevos_hilos);
        }
        else
        {
            list_remove_element(colas_prioridad, priority_queue);
            signal_mutex(&mutex_cola_de_prioridad);
            signal_sem(&sem_CPU_libre_para_hilo);
        }
    }
}

// Devuelve la cola con mayor prioridad
void *_min_priority(void *a, void *b)
{
    priority_queue *cola_A = (priority_queue *)a;
    priority_queue *cola_B = (priority_queue *)b;
    return cola_A->prioridad <= cola_B->prioridad ? cola_A : cola_B;
}
priority_queue *devolver_lista_maxima_prioridad()
{
    if (list_is_empty(colas_prioridad))
    {
        return NULL;
    }
    return list_get_minimum(colas_prioridad, _min_priority);
}

// Planificador Round Robin
void planificar_RR(t_queue *queue)
{
    tcb *hilo = queue_pop(queue);
    signal_mutex(&mutex_cola_de_prioridad);
    poner_en_ejecucion(hilo);
    pthread_t hilo_quantum_interrupt;
    pthread_create(&hilo_quantum_interrupt, NULL, quantum_interrupt, (void *)hilo);
    pthread_detach(hilo_quantum_interrupt);
}

void *interrumpir_por_fin_quantum(void *arg)
{
    while (1)
    {
        wait_sem(&hilo_interrumpido);
        wait_mutex(&mutex_hilo_en_ejecucion);
        informar_interrupcion_por_quantum_CPU(hilo_en_ejecucion);
        signal_mutex(&mutex_hilo_en_ejecucion);
    }
}

// FUNCIONES

// Pone en ejecucion el hilo dado
void poner_en_ejecucion(tcb *hilo)
{
    wait_mutex(&mutex_hilo_en_ejecucion);
    wait_mutex(&proceso_en_ejecucion_mutex);
    process_to_execute = hilo->proceso;
    hilo_en_ejecucion = hilo;
    enviar_hilo_exec_CPU(hilo);
    log_debug(kernel_logger, "## (%d:%d) - Estado: EXEC", hilo->proceso->pid, hilo->tid);
    signal_mutex(&mutex_hilo_en_ejecucion);
    signal_mutex(&proceso_en_ejecucion_mutex);
}

// Crea y mantiene actualizadas las colas de prioridad
void *crear_colas_por_prioridad(void *arg)
{
    while (1)
    {
        wait_sem(&nuevos_hilos_para_cola);
        wait_mutex(&creando_colas_por_prioridad);
        wait_mutex(&mutex_queue_ready_threads);
        if (!queue_is_empty(queue_ready_threads))
        {
            signal_mutex(&mutex_queue_ready_threads);
            tcb *hilo = pop_queue_ready();
            wait_mutex(&mutex_cola_de_prioridad);
            agregar_a_cola_prioridad(hilo);
        }
        else{
            signal_mutex(&mutex_queue_ready_threads);
        }
        signal_mutex(&creando_colas_por_prioridad);
    }
}

void agregar_a_cola_prioridad(tcb *hilo)
{
    if (exists_priority(hilo->prioridad))
    {
        add_thread_by_priority(hilo);
        signal_mutex(&mutex_cola_de_prioridad);
        return;
    }
    // Si no existe una cola para esta prioridad, crear una nueva
    create_priority_queue(hilo->prioridad);
    add_thread_by_priority(hilo);
    signal_mutex(&mutex_cola_de_prioridad);
    signal_sem(&nuevos_hilos);
}

// Existe cola de prioridad
bool exists_priority(int prioridad_hilo)
{
    bool _match_priority(void *elem)
    {
        priority_queue *cola = (priority_queue *)elem;
        return (cola->prioridad == prioridad_hilo);
    }
    return (list_find(colas_prioridad, _match_priority) != NULL);
}

// Agrega un hilo a la cola de prioridad correspondiente
void add_thread_by_priority(tcb *hilo)
{
    int prioridad_hilo = hilo->prioridad;
    bool _match_priority(void *elem)
    {
        priority_queue *cola = (priority_queue *)elem;
        return (cola->prioridad == prioridad_hilo);
    }
    priority_queue *cola = list_find(colas_prioridad, _match_priority);
    queue_push(cola->queue, hilo);
}

// Crea una cola de prioridad de forma ordenada
void create_priority_queue(int prioridad_hilo)
{
    priority_queue *cola = malloc(sizeof(priority_queue));
    cola->queue = queue_create();
    cola->prioridad = prioridad_hilo;
    bool _has_more_priority_than(void *a, void *b)
    {
        priority_queue *cola_A = (priority_queue *)a;
        priority_queue *cola_B = (priority_queue *)b;
        return cola_A->prioridad <= cola_B->prioridad;
    }
    list_add_sorted(colas_prioridad, cola, _has_more_priority_than);
}

// Quantum
void *quantum_interrupt(void *arg)
{
    tcb *hilo_llamado = ((tcb *)arg);
    //log_debug(kernel_logger, "Quantum iniciado");
    usleep(kernel_config.QUANTUM * 1000);
    //log_debug(kernel_logger, "Quantum finalizado");
    wait_sem(&sem_en_syscall);
    wait_mutex(&mutex_hilo_en_ejecucion);
    if (hilo_en_ejecucion == hilo_llamado) // no se hizo una syscall bloqueante
    {
        signal_sem(&sem_en_syscall);
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_sem(&hilo_interrumpido);
        return NULL;
    }
    signal_sem(&sem_en_syscall);
    signal_mutex(&mutex_hilo_en_ejecucion);
    return NULL;
}