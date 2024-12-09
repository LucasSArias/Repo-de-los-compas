#ifndef SYSCALLSH
#define SYSCALLSH

#include "bibliotecas.h"
#include "estructuras.h"

typedef struct IO_args {
    tcb* hilo;
    int milisegundos;
}IO_args;

typedef struct DUMP_args{
    tcb* hilo;
    int fd_conexion_memoria;
} DUMP_args;

void PROCESS_CREATE_KERNEL(char* pseudocodigo, int tam_memoria, int prioridad_hilo);
void THREAD_EXIT_KERNEL();

void* gestionar_syscalls(void* arg);

void liberar_tids_bloqueados(tcb* hilo);

void liberar_mutex_bloqueados(tcb* hilo);

void liberar_bloqueados(mutex_t* mutex);

t_paquete* recibir_paquete_syscall(int conexion);
syscall_args* recibir_PROCESS_CREATE(t_paquete* paquete, int offset);
syscall_args* recibir_THREAD_CREATE(t_paquete* paquete, int offset);
syscall_args* recibir_SYSCALL_IO(t_paquete* paquete, int offset);
syscall_args* recibir_SYSCALL_MUTEX(t_paquete* paquete, int offset);
syscall_args* recibir_TID_ARG(t_paquete* paquete, int offset);

void enviar_confirmacion_syscall();

void enviar_syscall_bloqueante(int tid, int pid, op_code syscall);

void free_syscall_args(syscall_args* args);
syscall_args *inicializar_syscalls_args();


#endif /* SYSCALLS_H */