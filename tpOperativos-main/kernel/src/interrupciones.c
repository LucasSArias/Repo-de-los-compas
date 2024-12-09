#include "interrupciones.h"
#include "inicializacion.h"
#include "conexiones.h"
#include "planificacion.h"

void *recibir_interrupciones_confirmadas(void *args)
{
    int crashes = 0;
    while (1)
    {
        if (crashes >= 3)
        {
            log_error(kernel_logger, "Al parecer se desconecto CPU. Terminando");
            exit(EXIT_FAILURE);
        }
        t_interrupcion *interrupcion_confirmada = recibir_interrupciones_confirmadas_cpu();
        switch (interrupcion_confirmada->reason)
        {
        case END_OF_QUANTUM:
            wait_mutex(&mutex_hilo_en_ejecucion);
            wait_mutex(&proceso_en_ejecucion_mutex);
            if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0 || strcmp(kernel_config.ALGORITMO_PLANIFICACION, "CMN") == 0)
            {
                wait_mutex(&creando_colas_por_prioridad);
                wait_mutex(&mutex_cola_de_prioridad);
                agregar_a_cola_prioridad(hilo_en_ejecucion);
                process_to_execute = NULL;
                hilo_en_ejecucion = NULL;
                signal_mutex(&mutex_hilo_en_ejecucion);
                signal_mutex(&proceso_en_ejecucion_mutex);
                log_info(kernel_logger, "## (%d:%d) - Desalojado por fin de Quantum", interrupcion_confirmada->pid, interrupcion_confirmada->tid);
                signal_mutex(&creando_colas_por_prioridad);
                signal_sem(&sem_CPU_libre_para_hilo);
                break;
            }
            process_to_execute = NULL;
            hilo_en_ejecucion = NULL;
            signal_mutex(&mutex_hilo_en_ejecucion);
            signal_mutex(&proceso_en_ejecucion_mutex);
            log_info(kernel_logger, "## (%d:%d) - Desalojado por fin de Quantum", interrupcion_confirmada->pid, interrupcion_confirmada->tid);
            signal_sem(&sem_CPU_libre_para_hilo);
            break;
        case END_BY_SEGMENTATION_FAULT:
            wait_mutex(&mutex_hilo_en_ejecucion);
            wait_mutex(&proceso_en_ejecucion_mutex);
            // log_debug(kernel_logger, "## (%d) - Llego interrupcion por segmentation fault ", process_to_execute->pid);
            finalizacion_procesos(process_to_execute);
            process_to_execute = NULL;
            hilo_en_ejecucion = NULL;
            signal_mutex(&mutex_hilo_en_ejecucion);
            signal_mutex(&proceso_en_ejecucion_mutex);
            signal_sem(&espacio_para_proceso);
            signal_sem(&sem_CPU_libre_para_hilo);
            break;
        case END_BY_PROCESS_EXIT:
            wait_mutex(&mutex_hilo_en_ejecucion);
            wait_mutex(&proceso_en_ejecucion_mutex);
            // log_debug(kernel_logger, "## (%d) - Llego interrupcion de fin proceso ", process_to_execute->pid);
            finalizacion_procesos(process_to_execute);
            process_to_execute = NULL;
            hilo_en_ejecucion = NULL;
            signal_mutex(&mutex_hilo_en_ejecucion);
            signal_mutex(&proceso_en_ejecucion_mutex);
            signal_sem(&sem_en_syscall);
            signal_sem(&sem_CPU_libre_para_hilo);
            break;
        case BLOCK_THREAD_JOIN:
        case END_BY_THREAD_CANCEL: // Esto solo pasa si justo el tid pasado es del mismo hilo que lo llamo (por las dudas)
        case END_BY_THREAD_EXIT:
        case BLOCK_IO:
        case BLOCK_BY_MUTEX:
            //log_info(kernel_logger, "## (%d:%d) - Llego interrupcion de bloqueo", interrupcion_confirmada->pid, interrupcion_confirmada->tid);
            wait_mutex(&mutex_hilo_en_ejecucion);
            wait_mutex(&proceso_en_ejecucion_mutex);
            process_to_execute = NULL;
            hilo_en_ejecucion = NULL;
            signal_mutex(&mutex_hilo_en_ejecucion);
            signal_mutex(&proceso_en_ejecucion_mutex);
            signal_sem(&sem_en_syscall);
            signal_sem(&sem_CPU_libre_para_hilo);
            break;
        case BLOCK_BY_DUMP:
            // log_debug(kernel_logger, "## (%d:%d) - Llego interrupcion de bloqueo por dump", interrupcion_confirmada->pid, interrupcion_confirmada->tid);
            wait_mutex(&mutex_hilo_en_ejecucion);
            wait_mutex(&proceso_en_ejecucion_mutex);
            process_to_execute = NULL;
            hilo_en_ejecucion = NULL;
            signal_mutex(&mutex_hilo_en_ejecucion);
            signal_mutex(&proceso_en_ejecucion_mutex);
            signal_sem(&sem_en_syscall);
            signal_sem(&sem_CPU_libre_para_hilo);
            signal_sem(&bloqueo_por_dump);
            // log_debug(kernel_logger, "signal de bloqueo por dump");
            break;
        case NOT_BLOCK_THREAD_JOIN:
        case NOT_BLOCK_BY_MUTEX:
        case NOT_END_BY_THREAD_CANCEL:
            break;
        default:
            log_error(kernel_logger, "Interrupcion no reconocida. Tipo: %d", interrupcion_confirmada->reason);
            crashes++;
            break;
        }
        free(interrupcion_confirmada);
    }
}

t_interrupcion *recibir_interrupciones_confirmadas_cpu()
{
    t_interrupcion *interrupcion_confirmada = malloc(sizeof(t_interrupcion));
    recv(fd_conexion_interrupt, &(interrupcion_confirmada->reason), sizeof(op_code), MSG_WAITALL);
    int size = 0;
    recv(fd_conexion_interrupt, &size, sizeof(int), MSG_WAITALL);
    recv(fd_conexion_interrupt, &(interrupcion_confirmada->pid), sizeof(uint32_t), MSG_WAITALL);
    recv(fd_conexion_interrupt, &(interrupcion_confirmada->tid), sizeof(uint32_t), MSG_WAITALL);
    return interrupcion_confirmada;
}
