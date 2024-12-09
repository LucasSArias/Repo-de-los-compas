#include "syscall.h"

static void _enviar_syscall_mutex(syscall_t codigo, char *recurso);
static void _enviar_syscall_identificador(syscall_t codigo);
static void _enviar_syscall_id_tid(syscall_t codigo, uint32_t tid);

void enviar_syscall_dump_memoria()
{
    _enviar_syscall_identificador(S_DUMP_MEMORY);
}

void enviar_syscall_process_create(char *archivo, int tamanio, int prioridad_tid_0)
{
    t_paquete *paquete = iniciar_paquete(SYSCALL);
    paquete->buffer->size = 3 * sizeof(uint32_t) + strlen(archivo);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    int operacion = S_PROCESS_CREATE;
    memcpy(paquete->buffer->stream + offset, &operacion, sizeof(syscall_t));
    offset += sizeof(syscall_t);
    memcpy(paquete->buffer->stream + offset, &(tamanio), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, &(prioridad_tid_0), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, archivo, strlen(archivo));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
}

void enviar_syscall_process_exit()
{
    _enviar_syscall_identificador(S_PROCESS_EXIT);
}

void enviar_syscall_thread_create(char *archivo, int prioridad)
{
    t_paquete *paquete = iniciar_paquete(SYSCALL);
    paquete->buffer->size = 2 * sizeof(uint32_t) + strlen(archivo);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    int operacion = S_THREAD_CREATE;
    memcpy(paquete->buffer->stream + offset, &operacion, sizeof(syscall_t));
    offset += sizeof(syscall_t);
    memcpy(paquete->buffer->stream + offset, &(prioridad), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, archivo, strlen(archivo));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
}

void enviar_syscall_thread_exit()
{
    _enviar_syscall_identificador(S_THREAD_EXIT);
}

void enviar_syscall_mutex_create(char *recurso)
{
    _enviar_syscall_mutex(S_MUTEX_CREATE, recurso);
}

void enviar_syscall_mutex_lock(char *recurso)
{
    _enviar_syscall_mutex(S_MUTEX_LOCK, recurso);
}

void enviar_syscall_mutex_unlock(char *recurso)
{
    _enviar_syscall_mutex(S_MUTEX_UNLOCK, recurso);
}

void enviar_syscall_io(uint32_t tiempo)
{
    t_paquete *paquete = iniciar_paquete(SYSCALL);
    paquete->buffer->size = sizeof(syscall_t) + sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    int operacion = S_IO;
    memcpy(paquete->buffer->stream + offset, &operacion, sizeof(syscall_t));
    offset += sizeof(syscall_t);
    memcpy(paquete->buffer->stream + offset, &(tiempo), sizeof(uint32_t));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
}


void enviar_syscall_thread_join(uint32_t tid)
{
    _enviar_syscall_id_tid(S_THREAD_JOIN, tid);
}

void enviar_syscall_thread_cancel(uint32_t tid)
{
    _enviar_syscall_id_tid(S_THREAD_CANCEL, tid);
}

void _enviar_syscall_mutex(syscall_t codigo, char *recurso)
{
    t_paquete *paquete = iniciar_paquete(SYSCALL);
    paquete->buffer->size = sizeof(syscall_t) + strlen(recurso);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(codigo), sizeof(syscall_t));
    offset += sizeof(syscall_t);
    memcpy(paquete->buffer->stream + offset, recurso, strlen(recurso));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
}

void _enviar_syscall_identificador(syscall_t codigo)
{
    t_paquete *paquete = iniciar_paquete(SYSCALL);
    paquete->buffer->size = sizeof(syscall_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(codigo), sizeof(syscall_t));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
}

void _enviar_syscall_id_tid(syscall_t codigo, uint32_t tid)
{
    t_paquete *paquete = iniciar_paquete(SYSCALL);
    paquete->buffer->size = sizeof(syscall_t) + sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(codigo), sizeof(syscall_t));
    offset += sizeof(syscall_t);
    memcpy(paquete->buffer->stream + offset, &(tid), sizeof(uint32_t));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
}