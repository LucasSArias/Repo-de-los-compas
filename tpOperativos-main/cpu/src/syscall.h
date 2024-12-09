#ifndef CPU_SYSCALL_H_
#define CPU_SYSCALL_H_

#include "bibliotecas.h"

#include "conexiones.h"
#include "cpu_dispatch.h"

void enviar_syscall_dump_memoria();
void enviar_syscall_process_create(char *archivo, int tamanio, int prioridad_tid_0);
void enviar_syscall_process_exit();
void enviar_syscall_thread_create(char *archivo, int prioridad);
void enviar_syscall_thread_exit();
void enviar_syscall_mutex_create(char *recurso);
void enviar_syscall_mutex_lock(char *recurso);
void enviar_syscall_mutex_unlock(char *recurso);
void enviar_syscall_io(uint32_t tiempo);
void enviar_syscall_thread_join(uint32_t tid);
void enviar_syscall_thread_cancel(uint32_t tid);

#endif /* CPU_SYSCALL_H_ */