#include "cpu_interrupt.h"

void* atender_cpu_interrupt( void* args){
    int crashes = 0;
    while (1)
    {
        if(crashes >= 3)
        {
            log_error(cpu_logger, "Al parecer se desconecto Kernel. Terminando");
            close(conexion_interrupt);
            exit(EXIT_FAILURE);
        }
        t_interrupcion interrupcion = recibir_interrupcion_kernel(); // Aca recibe la interrupcion de fin de quantum y ademas si la syscall termina en replanificar
        switch (interrupcion.reason)
        {
            case NOT_BLOCK_THREAD_JOIN:
            case NOT_BLOCK_BY_MUTEX:
            case NOT_END_BY_THREAD_CANCEL:
                pthread_mutex_lock(&mutex_var_global_syscall);
                interrupcion_syscall = interrupcion;
                hay_syscall_bloqueante = true;
                pthread_mutex_unlock(&mutex_var_global_syscall);
                log_debug(cpu_logger, "## Llega syscall no bloqueante");
                sem_post(&llego_la_interrupcion);
                break;
            case BLOCK_THREAD_JOIN:
            case END_BY_THREAD_CANCEL:
            case END_BY_THREAD_EXIT:
            case END_BY_PROCESS_EXIT:
            case BLOCK_IO:
            case BLOCK_BY_MUTEX:
            case BLOCK_BY_DUMP:
                log_debug(cpu_logger, "## Llega syscall bloquante");
                pthread_mutex_lock(&mutex_var_global_syscall);
                interrupcion_syscall = interrupcion;
                hay_syscall_bloqueante = true;
                pthread_mutex_unlock(&mutex_var_global_syscall);
                sem_post(&llego_la_interrupcion);
                break;
            case END_OF_QUANTUM:
                log_info(cpu_logger, "## Llega interrupción al puerto Interrupt");
                pthread_mutex_lock(&mutex_var_global_hay_interrupcion);
                interrupcion_quantum = interrupcion;
                hay_interrupcion = true;
                pthread_mutex_unlock(&mutex_var_global_hay_interrupcion);
                break;
            default:
                log_error(cpu_logger, "Interrupcion no reconocida. Tipo: %d", interrupcion.reason);
                crashes++;
                break;
        }
    }
}

t_interrupcion recibir_interrupcion_kernel()
{
    int reason = recibir_entero(conexion_interrupt);
    t_interrupcion interrupcion;
    interrupcion.reason = reason;
    t_buffer *buffer = malloc(sizeof(t_buffer));
    recv(conexion_interrupt, &(buffer->size), sizeof(int), MSG_WAITALL);
    buffer->stream = malloc(buffer->size);
    recv(conexion_interrupt, buffer->stream, buffer->size, MSG_WAITALL);

    int offset = 0;
    memcpy(&(interrupcion.pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(interrupcion.tid), buffer->stream + offset, sizeof(uint32_t));
    eliminar_buffer(buffer);
    return interrupcion;
}

void confirmar_interrupcion_kernel(t_interrupcion interrupcion)
{
    int tid = interrupcion.tid;
    int pid = interrupcion.pid;
    int reason = interrupcion.reason;
    enviar_TID_PID(tid, pid, conexion_interrupt, reason);
}
