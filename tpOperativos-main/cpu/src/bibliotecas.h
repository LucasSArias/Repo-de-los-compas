#ifndef CPU_BIBLIOTECAS_H
#define CPU_BIBLIOTECAS_H

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <utils/constants.h>


#include <utils/utils.h>

typedef struct
{
    char *ip_memoria;
    char *puerto_memoria;
    char *puerto_escucha_dispatch;
    char *puerto_escucha_interrupt;
    t_log_level log_level;
} t_cpu_config;

typedef enum Registro
{
    PC,
    AX,
    BX,
    CX,
    DX,
    EX,
    FX,
    GX,
    HX,
} e_registro;

typedef enum TipoInstruccion {
    CPU_SET,
    CPU_SUM,
    CPU_SUB ,
    CPU_READ_MEM ,
    CPU_WRITE_MEM ,
    CPU_JNZ ,
    CPU_LOG ,
    SYSCALL_MUTEX_CREATE ,
    SYSCALL_MUTEX_LOCK ,
    SYSCALL_MUTEX_UNLOCK ,
    SYSCALL_DUMP_MEMORY,
    SYSCALL_IO ,
    SYSCALL_PROCESS_CREATE ,
    SYSCALL_THREAD_CREATE,
    SYSCALL_THREAD_CANCEL,
    SYSCALL_THREAD_JOIN,
    SYSCALL_THREAD_EXIT,
    SYSCALL_PROCESS_EXIT
} e_tipo_instruccion;

typedef struct{
  op_code reason;
  uint32_t tid;
  uint32_t pid;
} t_interrupcion;

typedef struct 
{
    e_registro registro;
    uint32_t valor;
    e_registro registro_datos;
    e_registro registro_destino;
    e_registro registro_origen;
    e_registro registro_direccion;
    uint32_t tid;
    uint32_t tiempo;
    char *recurso;
    char *archivo;
    uint32_t prioridad;
    uint32_t prioridad_tid_0;
    uint32_t tamanio;
    uint32_t direccion_fisica;
    bool usar_mmu;
} t_generic_arguments;

typedef struct {
    char *codigo;
    e_tipo_instruccion tipo_instruccion;
    t_generic_arguments *args;
} t_instruccion;

typedef struct Hilo
{
    uint32_t pid;
    uint32_t tid;
} t_hilo;

typedef struct ContextoEjecucion
{
    uint32_t pc;
    uint32_t base;
    uint32_t limite;
    uint32_t ax;
    uint32_t bx;
    uint32_t cx;
    uint32_t dx;
    uint32_t ex;
    uint32_t fx;
    uint32_t gx;
    uint32_t hx;
} t_contexto_ejecucion;

extern pthread_mutex_t mutex_var_global_syscall;
extern pthread_mutex_t mutex_var_global_interrupt;
extern pthread_mutex_t mutex_var_global_hay_interrupcion;
extern sem_t llego_la_interrupcion;

#define TIPOS_REGISTROS 9
extern const char *enum_names_registros[TIPOS_REGISTROS];

#endif /* CPU_BIBLIOTECAS_H */