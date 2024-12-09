#include "conexiones.h"

t_log* cpu_logger;

static void _solicitar_contexto_de_ejecucion(t_hilo *hilo);
static void _solicitar_instruccion_a_memoria(uint32_t pid, uint32_t tid, uint32_t pc);
static char *_recibir_instruccion_de_memoria();

void iniciar_conexiones_cpu()
{
    iniciar_conexion_memoria();
    iniciar_servidor_cpu();
}

void cerrar_conexiones_cpu()
{
    close(conexion_dispatch);
    close(conexion_interrupt);
    close(conexion_memoria);
}

void iniciar_conexion_memoria()
{
    // Cliente de MEMORIA
    conexion_memoria = crear_conexion(cpu_config->ip_memoria, cpu_config->puerto_memoria);
    enviar_handshake(conexion_memoria, HANDSHAKE_CPU);
    op_code handshake_respuesta = recibir_entero(conexion_memoria);
    if (handshake_respuesta != HANDSHAKE_ACCEPTED)
    {
        log_error(cpu_logger, "No se pudo conectar con el servidor de MEMORIA");
        exit(1);
    }
    log_debug(cpu_logger, "Conexion con MEMORIA creada y establecida con exito");
}

void iniciar_servidor_cpu()
{
    iniciar_servidor_dispatch();
    iniciar_servidor_interrupt();
}

void iniciar_servidor_dispatch()
{
    int server_for_kernel = iniciar_servidor(cpu_config->puerto_escucha_dispatch);
    log_debug(cpu_logger, "Servidor listo para recibir Kernel dispatch");

    conexion_dispatch = esperar_cliente(server_for_kernel);

    op_code respuesta = recibir_entero(conexion_dispatch);
    if (respuesta != HANDSHAKE_KERNEL)
    {
        log_error(cpu_logger, "El cliente no es CPU");
        exit(1);
    }
    enviar_entero(conexion_dispatch, HANDSHAKE_ACCEPTED);
    log_debug(cpu_logger, "Conexion con DISPATCH establecida con exito");
}

void iniciar_servidor_interrupt()
{
    int server_for_kernel = iniciar_servidor(cpu_config->puerto_escucha_interrupt);
    log_debug(cpu_logger, "Servidor listo para recibir Kernel interrupt");

    conexion_interrupt = esperar_cliente(server_for_kernel);

    op_code respuesta = recibir_entero(conexion_interrupt);
    if (respuesta != HANDSHAKE_KERNEL)
    {
        log_error(cpu_logger, "El cliente no es CPU");
        exit(1);
    }
    enviar_entero(conexion_interrupt, HANDSHAKE_ACCEPTED);
    log_debug(cpu_logger, "Conexion con INTERRUPT establecida con exito");
}

void recibir_hilo_a_ejecutar_kernel()
{
    op_code op_code = recibir_entero(conexion_dispatch);
    if(op_code != EJECUTAR_HILO)
    {
        log_error(cpu_logger, "No se recibio la operacion para recibir el hilo a ejecutar");
        exit(EXIT_FAILURE);
    }
    
    t_buffer *buffer = malloc(sizeof(t_buffer));
    recv(conexion_dispatch, &(buffer->size), sizeof(int), MSG_WAITALL);
    buffer->stream = malloc(buffer->size);
    recv(conexion_dispatch, buffer->stream, buffer->size, MSG_WAITALL);
    
    hilo_ejecutar = malloc(sizeof(t_hilo));

    int offset = 0;
    memcpy(&(hilo_ejecutar->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&(hilo_ejecutar->tid), buffer->stream + offset, sizeof(uint32_t));

    eliminar_buffer(buffer);
}

void recibir_contexto_ejecucion_memoria()
{
    contexto_ejecucion = malloc(sizeof(t_contexto_ejecucion));

    _solicitar_contexto_de_ejecucion(hilo_ejecutar);
    
    int op_code = recibir_entero(conexion_memoria);
    if (op_code != OK)
    {
        log_error(cpu_logger, "No se pudo obtener el contexto de ejecucion");
        exit(1);
    }

    int size = 0;
    recv(conexion_memoria, &size, sizeof(int), MSG_WAITALL);
    if(size != sizeof(uint32_t) * 11)
    {
        log_warning(cpu_logger, "EL tamaño de la estructura recibida es incorrecto (tal vez)");
        //exit(1);
    }

    recv(conexion_memoria, &(contexto_ejecucion->ax), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->bx), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->cx), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->dx), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->ex), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->fx), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->gx), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->hx), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->pc), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->base), sizeof(uint32_t), MSG_WAITALL);
    recv(conexion_memoria, &(contexto_ejecucion->limite), sizeof(uint32_t), MSG_WAITALL);
}

char *recibir_instruccion_a_ejecutar_memoria(uint32_t pid, uint32_t tid, uint32_t pc)
{
    _solicitar_instruccion_a_memoria(pid, tid, pc);
    return _recibir_instruccion_de_memoria();
}

void _solicitar_instruccion_a_memoria(uint32_t pid, uint32_t tid, uint32_t pc)
{
    t_paquete *paquete = iniciar_paquete(GET_INSTRUCCION);
    paquete->buffer->size = 3 * sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(pid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(tid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(pc), sizeof(uint32_t));

    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void _solicitar_contexto_de_ejecucion(t_hilo *hilo)
{
    t_paquete *paquete = iniciar_paquete(GET_CONTEXTO_EJECUCION);
    paquete->buffer->size = 2 * sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(hilo->pid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(hilo->tid), sizeof(uint32_t));

    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

char *_recibir_instruccion_de_memoria()
{
    int op_code = recibir_entero(conexion_memoria);
    if(op_code != OK){
        log_error(cpu_logger, "No se pudo obtener la instruccion");
        exit(EXIT_FAILURE);
    }
    int size = 0;
    recv(conexion_memoria, &size, sizeof(int), MSG_WAITALL);
    char *instruccion = malloc(size+1);
    recv(conexion_memoria, instruccion, size, MSG_WAITALL);
    instruccion[size] = '\0';
    return instruccion;
}

void pedir_read_a_memoria(uint32_t direccion_fisica)
{
    t_paquete *paquete = iniciar_paquete(READ_MEM);
    paquete->buffer->size = sizeof(uint32_t) * 3;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(hilo_ejecutar->pid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(hilo_ejecutar->tid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(direccion_fisica), sizeof(uint32_t));
    
    
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void pedir_escribir_a_memoria(uint32_t valor, uint32_t direccion_fisica)
{
    t_paquete *paquete = iniciar_paquete(WRITE_MEM);
    paquete->buffer->size = sizeof(uint32_t) * 4;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(hilo_ejecutar->pid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(hilo_ejecutar->tid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(direccion_fisica), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(valor), sizeof(uint32_t));
    
    
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void enviar_contexto_hilo_a_memoria()
{
    log_info(cpu_logger, "## TID: %d - Actualizo Contexto Ejecución", hilo_ejecutar->tid);
    t_paquete *paquete = iniciar_paquete(UPD_CONTEXTO_EJECUCION);
    paquete->buffer->size = sizeof(uint32_t) * 11;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(hilo_ejecutar->pid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(hilo_ejecutar->tid), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->ax), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->bx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->cx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->dx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->ex), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->fx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->gx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->hx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_ejecucion->pc), sizeof(uint32_t));

    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    op_code resultado = recibir_entero(conexion_memoria);
    if(resultado != OK){
        log_error(cpu_logger, "No se pudo actualizar el contexto de ejecucion");
        exit(EXIT_FAILURE);
    }
}

void enviar_tid_y_motivo_a_kernel(uint32_t hilo, op_code motivo)
{
}

uint32_t recibir_valor_read_mem()
{
    int op_code = recibir_entero(conexion_memoria);
    if(op_code != OK){
        log_error(cpu_logger, "No se pudo obtener el valor de la memoria");
        exit(EXIT_FAILURE);
    }
    int size = 0;
    recv(conexion_memoria, &size, sizeof(int), MSG_WAITALL);
    uint32_t valor;
    recv(conexion_memoria, &valor, size, MSG_WAITALL);
    return valor;
}

