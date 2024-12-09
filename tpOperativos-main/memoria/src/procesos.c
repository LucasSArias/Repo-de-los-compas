#include "procesos.h"

void crear_proceso(int socket, t_proceso_paquete *paquete_proceso)
{
    bool resultado = false;
    pthread_mutex_lock(&partition_mutex);
    if (memoria_config->esquema == FIJAS)
    {
        resultado = asignar_memoria_fija(paquete_proceso->tamanio, paquete_proceso->pid);
    }
    else if (memoria_config->esquema == DINAMICAS)
    {
        resultado = asignar_memoria_dinamica(paquete_proceso->tamanio, paquete_proceso->pid);
    }
    else
    {
        log_error(memoria_logger, "Esquema de memoria no válido. Esquema: %d", memoria_config->esquema);
        exit(1);
    }
    loguear_particiones();
    pthread_mutex_unlock(&partition_mutex);

    if (resultado == false)
    {
        enviar_estado_solicitud(socket, ERROR);
        log_error(memoria_logger, "No se pudo asignar memoria para el proceso. Tamaño: %d, PID: %d", paquete_proceso->tamanio, paquete_proceso->pid);
        list_destroy_and_destroy_elements(paquete_proceso->instrucciones, instruccion_destroy);
        free(paquete_proceso);
        return;
    }

    //SE crea CONTEXTO DEL HILO 0
    t_hilo_paquete *hilo_paquete = malloc(sizeof(t_hilo_paquete));
    hilo_paquete->pid = paquete_proceso->pid;
    hilo_paquete->tid = 0;
    hilo_paquete->instrucciones = paquete_proceso->instrucciones;
    crear_contexto_hilo(hilo_paquete);

    enviar_estado_solicitud(socket, OK);
    free(paquete_proceso);
}

void finalizar_proceso(int pid)
{
    liberar_memoria_por_pid(pid);

    finalizar_hilos_por_pid(pid);
}