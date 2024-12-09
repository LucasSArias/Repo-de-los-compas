#include "bibliotecas.h"
#include "conexiones.h"
#include "estructuras.h"
#include "inicializacion.h"
#include "syscalls.h"


// Conexiones efimeras con memoria
int iniciar_conexion_memoria(){
    int fd_conexion_memoria;
    fd_conexion_memoria = crear_conexion(kernel_config.IP_MEMORIA, kernel_config.PUERTO_MEMORIA);
    send_cliente_identificacion(fd_conexion_memoria, HANDSHAKE_KERNEL);
    op_code resultado = recibir_confirmacion(fd_conexion_memoria);
    if(resultado != HANDSHAKE_ACCEPTED){
        log_error(kernel_logger, "No se pudo conectar con el servidor de MEMORIA");
        exit(EXIT_FAILURE);
    }
    return fd_conexion_memoria; // CAMBIAR A QUE TODAS USEN UN FD DISTINTO
}

void cerrar_conexion_memoria(int fd_conexion_memoria){
    close(fd_conexion_memoria);
}

/* INTERACCION CON MEMORIA PARA PLANIFICACION */

// Preguntar a memoria si hay o no espacio para la ejecucion de un proceso
bool espacio_para_proceso_en_memoria(pcb *proceso)
{
    int fd_conexion_memoria = iniciar_conexion_memoria();
    enviar_informacion_proceso(proceso, fd_conexion_memoria);
    op_code resultado = recibir_confirmacion(fd_conexion_memoria);
    cerrar_conexion_memoria(fd_conexion_memoria);
    return resultado == OK;
}

op_code recibir_confirmacion(int fd_conexion){
    return recibir_entero(fd_conexion);
}

// Enviar a memoria la informacion de un proceso
void enviar_informacion_proceso(pcb* proceso, int conexion){
    t_paquete *paquete = iniciar_paquete(CREATE_PROCESS);
    paquete->buffer->size = sizeof(int) * 2 + sizeof(char) * strlen(proceso->hilo0->pseudocodigo);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(proceso->pid), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, &(proceso->tam_memoria_pseudocodigo), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, proceso->hilo0->pseudocodigo, sizeof(char) * strlen(proceso->hilo0->pseudocodigo));
    
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void enviar_informacion_hilo(tcb* hilo, int conexion){
    t_paquete *paquete = iniciar_paquete(CREATE_THREAD);
    paquete->buffer->size = sizeof(int) * 2 + sizeof(char) * strlen(hilo->pseudocodigo);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(hilo->proceso->pid), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, &(hilo->tid), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, hilo->pseudocodigo, sizeof(char) * strlen(hilo->pseudocodigo));
    
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}


//////////////////////////////////////////////////////////////

void enviar_pid_finalizar(int socket_cliente, int numero)
{
    t_paquete *paquete = iniciar_paquete(END_PROCESS);
    paquete->buffer->size = sizeof(int);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, &numero, paquete->buffer->size);
    
    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
}

// Avisar a memoria que un proceso finalizo su ejecucion
void notificar_finalizacion_proceso_memoria_y_esperar_confirmacion(pcb* proceso){
    int fd_conexion_memoria = iniciar_conexion_memoria();
    enviar_pid_finalizar(fd_conexion_memoria, proceso->pid); 
    // signal_mutex(&proceso_en_ejecucion_mutex);
    op_code resultado = recibir_confirmacion(fd_conexion_memoria);
    if (resultado == OK)
    {
        cerrar_conexion_memoria(fd_conexion_memoria);
        return;
    } else {
        log_error(kernel_logger, "Error al recibir confirmacion de memoria");
        cerrar_conexion_memoria(fd_conexion_memoria);
        return;
    }
}

/* INTERACCION CON CPU PARA PLANIFICACION */

// Envia a CPU el hilo a ejecutar
void enviar_hilo_exec_CPU(tcb* hilo){
    enviar_TID_PID(hilo->tid, hilo->proceso->pid, fd_conexion_dispatch, EJECUTAR_HILO);
    log_debug(kernel_logger, "Hilo enviado a CPU para ejecucion");
}

// Envia a CPU la interrupcion de un hilo -> Por FIN DE QUANTUM.
void informar_interrupcion_por_quantum_CPU(tcb* hilo){
    enviar_TID_PID(hilo->tid, hilo->proceso->pid, fd_conexion_interrupt, END_OF_QUANTUM);
}

/* INTERACCION CON MEMORIA PARA SYSCALLS */

// Envia a memoria la finalizacion de un hilo -> Se usa en THREAD_CANCEL_KERNEL y THREAD_EXIT_KERNEL
void notificar_finalizacion_hilo_memoria(tcb* hilo){
    int fd_conexion_memoria = iniciar_conexion_memoria();
    enviar_TID_PID(hilo->tid, hilo->proceso->pid, fd_conexion_memoria, END_THREAD);
    cerrar_conexion_memoria(fd_conexion_memoria);
}

// Solicita a memoria un dump
void enviar_solicitud_dump(int pid, int tid, int fd_conexion_memoria)
{
    enviar_TID_PID(tid, pid, fd_conexion_memoria, MEMORY_DUMP);
}

// Recibe de memoria la confirmacion de un dump
bool recibir_confirmacion_dump(int fd_conexion){
    op_code confirmacion_dump= recibir_confirmacion(fd_conexion);
    if (confirmacion_dump == OK)
    {
        return true;
    } else if (confirmacion_dump == ERROR){
        return false;
    }  else {
        log_error(kernel_logger, "Error al recibir confirmacion de memoria");
        return false;
    }
    
}