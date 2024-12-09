#include "ejecucion_hilos.h"

bool hay_interrupcion = false;
bool hay_syscall_bloqueante = false;
bool hay_segmentation_fault = false;
t_interrupcion interrupcion_quantum;
t_interrupcion interrupcion_syscall;
t_interrupcion interrupcion_segmentation_fault;
/*
#define TIPOS_INSTRUCCION_ENUM 18
static char *enum_names_instrucciones[TIPOS_INSTRUCCION_ENUM] = {"CPU_SET", "CPU_SUM", "CPU_SUB", "CPU_READ_MEM",
    "CPU_WRITE_MEM", "CPU_JNZ", "CPU_LOG", "SYSCALL_MUTEX_CREATE", "SYSCALL_MUTEX_LOCK", "SYSCALL_MUTEX_UNLOCK",
    "SYSCALL_DUMP_MEMORY", "SYSCALL_IO", "SYSCALL_PROCESS_CREATE", "SYSCALL_THREAD_CREATE", "SYSCALL_THREAD_CANCEL",
    "SYSCALL_THREAD_JOIN", "SYSCALL_THREAD_EXIT", "SYSCALL_PROCESS_EXIT"};
*/

static void _inicializar_instruccion();
static void _liberar_instruccion();

void ejecutar_hilo()
{

    while (1)
    {
        _inicializar_instruccion();
        log_info(cpu_logger, "## TID: %d - FETCH - Program Counter: %d", hilo_ejecutar->tid, contexto_ejecucion->pc);
        fetch();

        if (instruccion->codigo == NULL)
        {
            log_error(cpu_logger, "No se pudo obtener la instruccion");
            exit(EXIT_FAILURE);
        }

        if (strcmp(instruccion->codigo, "FIN") == 0)
        {
            return;
        }

        decode();

        execute();

        if (instruccion->tipo_instruccion != CPU_JNZ)
        {
            contexto_ejecucion->pc = contexto_ejecucion->pc + 1;
        }

        _liberar_instruccion();
        log_debug(cpu_logger, "Iniciando check_interrupt");
        if (!check_interrupt())
        {
            return;
        }
        log_debug(cpu_logger, "Fin de check_interrupt");
    }
}

void fetch()
{
    instruccion->codigo = recibir_instruccion_a_ejecutar_memoria(hilo_ejecutar->pid, hilo_ejecutar->tid, contexto_ejecucion->pc);
}

void decode()
{
    char **codigo_instruccion = string_split(instruccion->codigo, " ");
    if (string_equals_ignore_case(codigo_instruccion[0], "set"))
    {
        instruccion->tipo_instruccion = CPU_SET;
        instruccion->args->registro = convertir_a_registro(codigo_instruccion[1]);
        instruccion->args->valor = atoi(codigo_instruccion[2]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "sum"))
    {
        instruccion->tipo_instruccion = CPU_SUM;
        instruccion->args->registro_destino = convertir_a_registro(codigo_instruccion[1]);
        instruccion->args->registro_origen = convertir_a_registro(codigo_instruccion[2]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "sub"))
    {
        instruccion->tipo_instruccion = CPU_SUB;
        instruccion->args->registro_destino = convertir_a_registro(codigo_instruccion[1]);
        instruccion->args->registro_origen = convertir_a_registro(codigo_instruccion[2]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "read_mem"))
    {
        instruccion->tipo_instruccion = CPU_READ_MEM;
        instruccion->args->registro_datos = convertir_a_registro(codigo_instruccion[1]);
        instruccion->args->registro_direccion = convertir_a_registro(codigo_instruccion[2]);
        instruccion->args->usar_mmu = true;
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "write_mem"))
    {
        instruccion->tipo_instruccion = CPU_WRITE_MEM;
        instruccion->args->registro_direccion = convertir_a_registro(codigo_instruccion[1]);
        instruccion->args->registro_datos = convertir_a_registro(codigo_instruccion[2]);
        instruccion->args->usar_mmu = true;
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "jnz"))
    {
        instruccion->tipo_instruccion = CPU_JNZ;
        instruccion->args->registro = convertir_a_registro(codigo_instruccion[1]);
        instruccion->args->valor = atoi(codigo_instruccion[2]); // Verificar si es una instruccion o su numero
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "log"))
    {
        instruccion->tipo_instruccion = CPU_LOG;
        instruccion->args->registro = convertir_a_registro(codigo_instruccion[1]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "dump_memory"))
    {
        instruccion->tipo_instruccion = SYSCALL_DUMP_MEMORY;
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "io"))
    {
        instruccion->tipo_instruccion = SYSCALL_IO;
        instruccion->args->tiempo = atoi(codigo_instruccion[1]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "process_create"))
    {
        instruccion->tipo_instruccion = SYSCALL_PROCESS_CREATE;
        instruccion->args->archivo = string_duplicate(codigo_instruccion[1]);
        instruccion->args->tamanio = atoi(codigo_instruccion[2]);
        instruccion->args->prioridad_tid_0 = atoi(codigo_instruccion[3]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "thread_create"))
    {
        instruccion->tipo_instruccion = SYSCALL_THREAD_CREATE;
        instruccion->args->archivo = string_duplicate(codigo_instruccion[1]);
        instruccion->args->prioridad = atoi(codigo_instruccion[2]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "thread_join"))
    {
        instruccion->tipo_instruccion = SYSCALL_THREAD_JOIN;
        instruccion->args->tid = atoi(codigo_instruccion[1]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "thread_cancel"))
    {
        instruccion->tipo_instruccion = SYSCALL_THREAD_CANCEL;
        instruccion->args->tid = atoi(codigo_instruccion[1]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "mutex_create"))
    {
        instruccion->tipo_instruccion = SYSCALL_MUTEX_CREATE;
        instruccion->args->recurso = string_duplicate(codigo_instruccion[1]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "mutex_lock"))
    {
        instruccion->tipo_instruccion = SYSCALL_MUTEX_LOCK;
        instruccion->args->recurso = string_duplicate(codigo_instruccion[1]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "mutex_unlock"))
    {
        instruccion->tipo_instruccion = SYSCALL_MUTEX_UNLOCK;
        instruccion->args->recurso = string_duplicate(codigo_instruccion[1]);
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "thread_exit"))
    {
        instruccion->tipo_instruccion = SYSCALL_THREAD_EXIT;
    }
    else if (string_equals_ignore_case(codigo_instruccion[0], "process_exit"))
    {
        instruccion->tipo_instruccion = SYSCALL_PROCESS_EXIT;
    }
    else
    {
        log_error(cpu_logger, "Instruccion no reconocida");
    }
    string_array_destroy(codigo_instruccion);
}

void execute()
{
    switch (instruccion->tipo_instruccion)
    {
    case CPU_SET:
        log_info(cpu_logger, "## TID: %d - Ejecutando: SET - Registro: %s, Valor: %d", hilo_ejecutar->tid, enum_names_registros[instruccion->args->registro], instruccion->args->valor);
        setear_registro(instruccion->args->registro, instruccion->args->valor);
        break;
    case CPU_READ_MEM:
        log_info(cpu_logger, "## TID: %d - Ejecutando: READ_MEM - Registro: %s, Direccion: %s", hilo_ejecutar->tid, enum_names_registros[instruccion->args->registro_datos], enum_names_registros[instruccion->args->registro_direccion]);
        leer_memoria(instruccion->args->registro_datos, instruccion->args->registro_direccion);
        break;
    case CPU_WRITE_MEM:
        log_info(cpu_logger, "## TID: %d - Ejecutando: WRITE_MEM - Registro: %s, Direccion: %s", hilo_ejecutar->tid, enum_names_registros[instruccion->args->registro_datos], enum_names_registros[instruccion->args->registro_direccion]);
        escribir_memoria(instruccion->args->registro_datos, instruccion->args->registro_direccion);
        break;
    case CPU_SUM:
        log_info(cpu_logger, "## TID: %d - Ejecutando: SUM - Registro: %s, Registro: %s", hilo_ejecutar->tid, enum_names_registros[instruccion->args->registro_destino], enum_names_registros[instruccion->args->registro_origen]);
        sumar_registros(instruccion->args->registro_destino, instruccion->args->registro_origen);
        break;
    case CPU_SUB:
        log_info(cpu_logger, "## TID: %d - Ejecutando: SUB - Registro: %s, Registro: %s", hilo_ejecutar->tid, enum_names_registros[instruccion->args->registro_destino], enum_names_registros[instruccion->args->registro_origen]);
        restar_registros(instruccion->args->registro_destino, instruccion->args->registro_origen);
        break;
    case CPU_JNZ:
        log_info(cpu_logger, "## TID: %d - Ejecutando: JNZ - Registro: %s, Valor: %d", hilo_ejecutar->tid, enum_names_registros[instruccion->args->registro], instruccion->args->valor);
        execute_jnz(instruccion->args->registro, instruccion->args->valor);
        break;
    case CPU_LOG:
        log_info(cpu_logger, "## TID: %d - Ejecutando: LOG - Registro: %s", hilo_ejecutar->tid, enum_names_registros[instruccion->args->registro]);
        loggear_registro(instruccion->args->registro);
        break;
    case SYSCALL_DUMP_MEMORY:
        log_info(cpu_logger, "## TID: %d - Ejecutando: DUMP_MEMORY", hilo_ejecutar->tid);
        enviar_syscall_dump_memoria();
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_IO:
        log_info(cpu_logger, "## TID: %d - Ejecutando: IO - Tiempo: %d", hilo_ejecutar->tid, instruccion->args->tiempo);
        enviar_syscall_io(instruccion->args->tiempo);
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_PROCESS_CREATE:
        log_info(cpu_logger, "## TID: %d - Ejecutando: PROCESS_CREATE - Archivo: %s, Tamaño: %d, Prioridad: %d", hilo_ejecutar->tid, instruccion->args->archivo, instruccion->args->tamanio, instruccion->args->prioridad_tid_0);
        enviar_syscall_process_create(instruccion->args->archivo, instruccion->args->tamanio, instruccion->args->prioridad_tid_0);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_THREAD_CREATE:
        log_info(cpu_logger, "## TID: %d - Ejecutando: THREAD_CREATE - Archivo: %s, Prioridad: %d", hilo_ejecutar->tid, instruccion->args->archivo, instruccion->args->prioridad);
        enviar_syscall_thread_create(instruccion->args->archivo, instruccion->args->prioridad);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_THREAD_JOIN:
        log_info(cpu_logger, "## TID: %d - Ejecutando: THREAD_JOIN - TID: %d", hilo_ejecutar->tid, instruccion->args->tid);
        enviar_syscall_thread_join(instruccion->args->tid);
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_THREAD_CANCEL:
        log_info(cpu_logger, "## TID: %d - Ejecutando: THREAD_CANCEL - TID: %d", hilo_ejecutar->tid, instruccion->args->tid);
        enviar_syscall_thread_cancel(instruccion->args->tid);
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_MUTEX_CREATE:
        log_info(cpu_logger, "## TID: %d - Ejecutando: MUTEX_CREATE - Recurso: %s", hilo_ejecutar->tid, instruccion->args->recurso);
        enviar_syscall_mutex_create(instruccion->args->recurso);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_MUTEX_LOCK:
        log_info(cpu_logger, "## TID: %d - Ejecutando: MUTEX_LOCK - Recurso: %s", hilo_ejecutar->tid, instruccion->args->recurso);
        enviar_syscall_mutex_lock(instruccion->args->recurso);
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_MUTEX_UNLOCK:
        log_info(cpu_logger, "## TID: %d - Ejecutando: MUTEX_UNLOCK - Recurso: %s", hilo_ejecutar->tid, instruccion->args->recurso);
        enviar_syscall_mutex_unlock(instruccion->args->recurso);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_THREAD_EXIT:
        log_info(cpu_logger, "## TID: %d - Ejecutando: THREAD_EXIT", hilo_ejecutar->tid);
        enviar_syscall_thread_exit();
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        recibir_confirmacion_syscall();
        break;
    case SYSCALL_PROCESS_EXIT:
        log_info(cpu_logger, "## TID: %d - Ejecutando: PROCESS_EXIT", hilo_ejecutar->tid);
        enviar_syscall_process_exit();
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = true;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        recibir_confirmacion_syscall();
        break;
    default:
        log_error(cpu_logger, "Instruccion no reconocida. Tipo: %d", instruccion->tipo_instruccion);
        break;
    }
}

bool check_interrupt() // Aca veo como una interrupcion una syscall bloqueante
{
    pthread_mutex_lock(&mutex_var_global_syscall);
    bool hay_syscall_bloqueante_aux = hay_syscall_bloqueante;
    bool hay_segmentation_fault_aux = hay_segmentation_fault;
    pthread_mutex_unlock(&mutex_var_global_syscall);
    if (hay_syscall_bloqueante_aux)
    {
        //log_debug(cpu_logger, "## asfasdfasdfasfd");
        sem_wait(&llego_la_interrupcion);
        //log_debug(cpu_logger, "## Llega fdsafasdfas");
        if (interrupcion_syscall.reason != NOT_BLOCK_THREAD_JOIN && interrupcion_syscall.reason != NOT_BLOCK_BY_MUTEX && interrupcion_syscall.reason != NOT_END_BY_THREAD_CANCEL)
        {
            enviar_contexto_hilo_a_memoria();
            pthread_mutex_lock(&mutex_var_global_syscall);
            log_debug(cpu_logger, "## Confirmando interrupcion => %d", interrupcion_syscall.reason);
            confirmar_interrupcion_kernel(interrupcion_syscall);
            hay_syscall_bloqueante = false;
            pthread_mutex_unlock(&mutex_var_global_syscall);
            pthread_mutex_lock(&mutex_var_global_hay_interrupcion);
            hay_interrupcion = false;
            pthread_mutex_unlock(&mutex_var_global_hay_interrupcion);
            liberar_ejecucion();
            atender_cpu_dispatch();
            return false;
        }
        confirmar_interrupcion_kernel(interrupcion_syscall);
        hay_syscall_bloqueante = false;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        //log_debug(cpu_logger, "## Llega syscall no bloqueanteeeeeeeeeee");
    }
    if (hay_segmentation_fault_aux)
    {
        enviar_contexto_hilo_a_memoria();
        t_interrupcion interrupcion_segmentation_fault = inicializar_segmentation_fault();
        confirmar_interrupcion_kernel(interrupcion_segmentation_fault);
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = false;
        hay_segmentation_fault = false;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        pthread_mutex_lock(&mutex_var_global_hay_interrupcion);
        hay_interrupcion = false;
        pthread_mutex_unlock(&mutex_var_global_hay_interrupcion);
        liberar_ejecucion();
        atender_cpu_dispatch();
        return false;
    }
    pthread_mutex_lock(&mutex_var_global_hay_interrupcion);
    bool hay_interrupcion_aux = hay_interrupcion;
    pthread_mutex_unlock(&mutex_var_global_hay_interrupcion);
    if (hay_interrupcion_aux)
    {
        log_info(cpu_logger, "## Llega interrupción al puerto Interrupt");
        enviar_contexto_hilo_a_memoria();
        pthread_mutex_lock(&mutex_var_global_hay_interrupcion);
        confirmar_interrupcion_kernel(interrupcion_quantum);
        hay_interrupcion = false;
        pthread_mutex_unlock(&mutex_var_global_hay_interrupcion);
        pthread_mutex_lock(&mutex_var_global_syscall);
        hay_syscall_bloqueante = false;
        pthread_mutex_unlock(&mutex_var_global_syscall);
        liberar_ejecucion();
        atender_cpu_dispatch();
        return false;
    }
    return true;
}

t_interrupcion inicializar_segmentation_fault()
{
    t_interrupcion interrupcion;
    interrupcion.tid = hilo_ejecutar->tid;
    interrupcion.pid = hilo_ejecutar->pid;
    interrupcion.reason = END_BY_SEGMENTATION_FAULT;
    return interrupcion;
}

void recibir_confirmacion_syscall()
{
    int op_code = recibir_entero(conexion_dispatch);
    if (op_code != OK)
    {
        log_error(cpu_logger, "No se recibio la confirmacion por parte de Kernel");
        exit(EXIT_FAILURE);
    }
}

e_registro convertir_a_registro(char *codigo_registro)
{
    if (string_equals_ignore_case(codigo_registro, "PC"))
        return PC;
    else if (string_equals_ignore_case(codigo_registro, "AX"))
        return AX;
    else if (string_equals_ignore_case(codigo_registro, "BX"))
        return BX;
    else if (string_equals_ignore_case(codigo_registro, "CX"))
        return CX;
    else if (string_equals_ignore_case(codigo_registro, "DX"))
        return DX;
    else if (string_equals_ignore_case(codigo_registro, "EX"))
        return EX;
    else if (string_equals_ignore_case(codigo_registro, "FX"))
        return FX;
    else if (string_equals_ignore_case(codigo_registro, "GX"))
        return GX;
    else if (string_equals_ignore_case(codigo_registro, "HX"))
        return HX;
    else
    {
        log_error(cpu_logger, "Registro no reconocido");
        exit(1);
    }
}

void _inicializar_instruccion()
{
    instruccion = malloc(sizeof(t_instruccion));
    instruccion->codigo = NULL;
    instruccion->args = malloc(sizeof(t_generic_arguments));
    instruccion->args->usar_mmu = false;
    instruccion->args->recurso = NULL;
    instruccion->args->archivo = NULL;
}

void _liberar_instruccion()
{
    if (instruccion)
    {
        if (instruccion->codigo)
        {
            free(instruccion->codigo);
        }
        if (instruccion->args)
        {
            if (instruccion->args->recurso)
            {
                free(instruccion->args->recurso);
            }
            if (instruccion->args->archivo)
            {
                free(instruccion->args->archivo);
            }
            free(instruccion->args);
        }
        free(instruccion);
    }
}