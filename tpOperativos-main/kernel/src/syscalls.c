#include "syscalls.h"
#include "estructuras.h"
#include "inicializacion.h"
#include "conexiones.h"
#include "planificacion.h"

// SYSCALLS

// Crea un nuevo proceso.
void PROCESS_CREATE_KERNEL(char* pseudocodigo, int tam_memoria, int prioridad_hilo){
    signal_sem(&sem_en_syscall);
    pcb* proceso = crear_pcb(tam_memoria);
    wait_mutex(&proceso_en_ejecucion_mutex);
    tcb* hilo_0 = crear_tcb(0, prioridad_hilo, pseudocodigo);
    signal_mutex(&proceso_en_ejecucion_mutex);
    proceso->hilo0 = hilo_0;
    hilo_0->proceso = proceso;
    int *cero = malloc(sizeof(int));
    *cero = 0;
    list_add(proceso->tid_list, cero);
    push_queue_new(proceso);
    log_info(kernel_logger, "## (%d:0) Se crea el proceso - Estado: NEW", proceso->pid);
    signal_sem(&proceso_nuevo);
}

// Finaliza el proceso en ejecucion.
void PROCESS_EXIT_KERNEL(){
    wait_mutex(&proceso_en_ejecucion_mutex);
    pcb* proceso = process_to_execute;
    enviar_syscall_bloqueante(0, proceso->pid, END_BY_PROCESS_EXIT);
    log_info(kernel_logger, "## Finaliza el proceso %d", proceso->pid);
    signal_mutex(&proceso_en_ejecucion_mutex);
}

// Crea un nuevo hilo perteneciente al proceso en ejecucion.
void THREAD_CREATE_KERNEL(char* pseudocodigo, int prioridad_hilo){
    signal_sem(&sem_en_syscall);
    wait_mutex(&proceso_en_ejecucion_mutex);
    pcb* proceso = process_to_execute;
    int tid = list_size(proceso->tid_list);
    int *tid_prox = malloc(sizeof(int));
    *tid_prox = tid;
    tcb* hilo = crear_tcb(*tid_prox, prioridad_hilo, pseudocodigo);
    list_add(proceso->tid_list, tid_prox);
    signal_mutex(&proceso_en_ejecucion_mutex);
    push_queue_ready(hilo);
    int fd_conexion_memoria = iniciar_conexion_memoria();
    enviar_informacion_hilo(hilo, fd_conexion_memoria);
    op_code respuesta = recibir_entero(fd_conexion_memoria);
    if(respuesta != OK){
        log_error(kernel_logger, "No se pudo crear el hilo");
        exit(EXIT_FAILURE);
    }
    cerrar_conexion_memoria(fd_conexion_memoria);
    log_info(kernel_logger, "## (%d:%d) Se crea el Hilo - Estado: READY", hilo->proceso->pid, hilo->tid);
}

// Bloquea al hilo que invoca la syscall hasta que el hilo con el TID pasado por parametro finalice.
void THREAD_JOIN_KERNEL(int tid){
    wait_mutex(&proceso_en_ejecucion_mutex);
    tcb* hilo_bloqueante = buscar_tcb_por_tid_y_pid(tid, process_to_execute->pid);
    signal_mutex(&proceso_en_ejecucion_mutex);
    wait_mutex(&mutex_hilo_en_ejecucion);
    log_debug(kernel_logger, "## (%d:%d) - Bloqueante: PTHREAD_JOIN", process_to_execute->pid, tid);
    wait_mutex(&mutex_exit_threads);
    if (hilo_bloqueante == NULL || member_of_thread_queue(queue_exit_threads, tid)) {
        signal_mutex(&mutex_exit_threads);
        enviar_syscall_bloqueante(hilo_en_ejecucion->tid, hilo_en_ejecucion->proceso->pid, NOT_BLOCK_THREAD_JOIN);
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_sem(&sem_en_syscall);
        return;
    }
    signal_mutex(&mutex_exit_threads);
    tcb* hilo = hilo_en_ejecucion;
    enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, BLOCK_THREAD_JOIN);
    list_add_blocked(hilo);
    wait_mutex(&hilos_bloqueados_thread_join_mtx);
    list_add(hilo_bloqueante->hilos_bloqueados, hilo); //agrega a lista de bloqueados del hilo bloqueante
    signal_mutex(&hilos_bloqueados_thread_join_mtx);
    log_info(kernel_logger, "## (%d:%d) - Bloqueado por: PTHREAD_JOIN", hilo->proceso->pid, hilo->tid);
    signal_mutex(&mutex_hilo_en_ejecucion);
}

// Finaliza el hilo del tid pasado por parametro.
/*
THREAD_CANCEL, esta syscall recibe como parámetro un TID con el objetivo de finalizarlo pasando al mismo al estado EXIT. Se deberá indicar a la
 Memoria la finalización de dicho hilo. En caso de que el TID pasado por parámetro no exista o ya haya finalizado, esta syscall no hace nada. 
 Finalmente, el hilo que la invocó continuará su ejecución.
*/
void THREAD_CANCEL_KERNEL(int tid){
    wait_mutex(&proceso_en_ejecucion_mutex);
    pcb* proceso = process_to_execute;  
    tcb* hilo = buscar_tcb_por_tid_y_pid(tid, proceso->pid);
    signal_mutex(&proceso_en_ejecucion_mutex);
    if (hilo == NULL || member_of_thread_queue(queue_exit_threads, tid)) {
        enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, NOT_END_BY_THREAD_CANCEL);
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_sem(&sem_en_syscall);
        return;
    }
    wait_mutex(&mutex_hilo_en_ejecucion);
    if (hilo == hilo_en_ejecucion) {
        enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, END_BY_THREAD_CANCEL);
        log_info(kernel_logger, "## (%d:%d) Finaliza el hilo", hilo->proceso->pid, hilo->tid);
        notificar_finalizacion_hilo_memoria(hilo);
        push_queue_exit(hilo);
        liberar_tids_bloqueados(hilo);
        liberar_mutex_bloqueados(hilo);
        signal_mutex(&mutex_hilo_en_ejecucion);
        return;
    }
    enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, NOT_END_BY_THREAD_CANCEL);
    notificar_finalizacion_hilo_memoria(hilo);
    push_queue_exit(hilo);
    liberar_tids_bloqueados(hilo);
    liberar_mutex_bloqueados(hilo);
    log_info(kernel_logger, "## (%d:%d) Finaliza el hilo", hilo->proceso->pid, hilo->tid);
    signal_mutex(&mutex_hilo_en_ejecucion);
    signal_sem(&sem_en_syscall);
}

// Finaliza el hilo que invoca la syscall.
void THREAD_EXIT_KERNEL(){
    wait_mutex(&mutex_hilo_en_ejecucion);
    tcb* hilo = hilo_en_ejecucion;
    enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, END_BY_THREAD_EXIT);
    notificar_finalizacion_hilo_memoria(hilo);
    push_queue_exit(hilo);
    liberar_tids_bloqueados(hilo);
    liberar_mutex_bloqueados(hilo);
    log_info(kernel_logger, "## (%d:%d) Finaliza el hilo", hilo->proceso->pid, hilo->tid);
    signal_mutex(&mutex_hilo_en_ejecucion);
}

void liberar_tids_bloqueados(tcb* hilo_bloqueante){ // AVERIGUAR TEMA DE BLOQUEADOS POR MUTEX, (como hacerlo)
    tcb* hilo_auxiliar;
    while(!list_is_empty(hilo_bloqueante->hilos_bloqueados)){
        wait_mutex(&hilos_bloqueados_thread_join_mtx);
        hilo_auxiliar = list_get(hilo_bloqueante->hilos_bloqueados, 0);
        list_remove(hilo_bloqueante->hilos_bloqueados, 0);
        signal_mutex(&hilos_bloqueados_thread_join_mtx);
        list_remove_blocked(hilo_auxiliar);
        push_queue_ready(hilo_auxiliar);
    }
}

void liberar_mutex_bloqueados(tcb* hilo){
    for (int i = 0; i < list_size(mutex_lista); i++) {
        mutex_t* mutex = list_get(mutex_lista, i);
        if (mutex->hilo_duenio == hilo) {
            liberar_bloqueados(mutex);
        }
    }
}

void liberar_bloqueados(mutex_t* mutex){
    tcb* hilo;
    while(!queue_is_empty(mutex->bloqueados)){
        hilo = queue_pop(mutex->bloqueados);
        list_remove_blocked(hilo);
        push_queue_ready(hilo);
    }
    mutex->disponible = 1;
}

// Crea un nuevo mutex para el proceso sin asignarlo a ningún hilo.
void MUTEX_CREATE_KERNEL(char* nombre){
    signal_sem(&sem_en_syscall);
    mutex_t* nuevo_mutex = malloc(sizeof(mutex_t));

    if (nuevo_mutex == NULL) {  // Verifica si malloc falló
        log_error(kernel_logger, "Fallo al asignar memoria mutex: %s", nombre);
        return;  // Retornar NULL en caso de error
    }
    nuevo_mutex->disponible = 1; // como todo mutex lo inicializo en 1
    nuevo_mutex->mutex_nombre = string_duplicate(nombre);
    nuevo_mutex->bloqueados = queue_create();
    list_add(mutex_lista, nuevo_mutex);
}

// Asignar hilo al mutex o bloquear hilo
void MUTEX_LOCK_KERNEL(char* nombre){
    mutex_t* mutex = buscar_mutex_por_nombre(nombre);
    wait_mutex(&mutex_hilo_en_ejecucion);
    wait_mutex(&proceso_en_ejecucion_mutex);
    tcb* hilo = hilo_en_ejecucion;
    if(mutex != NULL && mutex->disponible == 1){ // Existe y no esta tomado
        enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, NOT_BLOCK_BY_MUTEX);
        signal_sem(&sem_en_syscall);
        mutex->hilo_duenio = hilo;
        mutex->disponible = 0;
        //log_debug(kernel_logger, "## (%d:%d) - Mutex asignado", hilo->proceso->pid, hilo->tid);
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_mutex(&proceso_en_ejecucion_mutex);
        return;
    } else if(mutex != NULL && mutex->disponible == 0){ // esta tomado
        enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, BLOCK_BY_MUTEX);
        queue_push(mutex->bloqueados, hilo); 
        list_add_blocked(hilo);
        log_info(kernel_logger, "## (%d:%d) bloqueado por MUTEX", hilo->proceso->pid, hilo->tid);
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_mutex(&proceso_en_ejecucion_mutex);
        return;
    }
    // Enviar hilo a exit
    enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, BLOCK_BY_MUTEX);
    push_queue_exit(hilo);
    liberar_tids_bloqueados(hilo);
    liberar_mutex_bloqueados(hilo);
    log_info(kernel_logger, "## (%d:%d) Finaliza el hilo", hilo->proceso->pid, hilo->tid);
    signal_mutex(&mutex_hilo_en_ejecucion);
    signal_mutex(&proceso_en_ejecucion_mutex);
}

// Desbloquea el primer hilo del mutex y se lo asigna.
void MUTEX_UNLOCK_KERNEL(char* nombre){
    mutex_t* mutex = buscar_mutex_por_nombre(nombre);
    wait_mutex(&mutex_hilo_en_ejecucion);
    wait_mutex(&proceso_en_ejecucion_mutex);
    tcb* hilo = hilo_en_ejecucion;
    if(mutex != NULL && mutex->disponible == 0 && mutex->hilo_duenio == hilo){
        signal_sem(&sem_en_syscall);
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_mutex(&proceso_en_ejecucion_mutex);
        if(queue_size(mutex->bloqueados) > 0){
            tcb* hilo_bloqueado = queue_pop(mutex->bloqueados); 
            list_remove_blocked(hilo_bloqueado);
            push_queue_ready(hilo_bloqueado);
            mutex->hilo_duenio = hilo_bloqueado;
        } else {
            mutex->disponible = 1;
            mutex->hilo_duenio = NULL;
        }
        return;
    }
    if (mutex == NULL)
    {
        enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, BLOCK_BY_MUTEX);
        push_queue_exit(hilo);
        liberar_tids_bloqueados(hilo);
        liberar_mutex_bloqueados(hilo);
        log_info(kernel_logger, "## (%d:%d) Finaliza el hilo", hilo->proceso->pid, hilo->tid);
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_mutex(&proceso_en_ejecucion_mutex);
        return;
    }
    signal_sem(&sem_en_syscall);
    signal_mutex(&mutex_hilo_en_ejecucion);
    signal_mutex(&proceso_en_ejecucion_mutex);
}

// Hace un dump del proceso
void *dump(void *args){
    DUMP_args* argumentos = ((DUMP_args*)args);
    tcb* hilo = argumentos->hilo;
    //log_debug(kernel_logger, "## (%d:%d) - DUMP ###########################", hilo->proceso->pid, hilo->tid);
    int fd_conexion_memoria = argumentos->fd_conexion_memoria;    
    //log_debug(kernel_logger, "## (%d:%d) - esperando confirmacion DUMP", hilo->proceso->pid, hilo->tid);
    if (recibir_confirmacion_dump(fd_conexion_memoria)) {
        cerrar_conexion_memoria(fd_conexion_memoria);
        // Desbloqueo del hilo
        wait_sem(&bloqueo_por_dump); 
        list_remove_blocked(hilo);
        push_queue_ready(hilo);
        //log_debug(kernel_logger, "## (%d:%d) finalizó DUMP y pasa a READY", hilo->proceso->pid, hilo->tid);
    } else {
        cerrar_conexion_memoria(fd_conexion_memoria);
        // Enviar hilo a exit
        wait_sem(&bloqueo_por_dump); 
        wait_mutex(&proceso_en_ejecucion_mutex);
        log_warning(kernel_logger, "No hay suficiente espacio para crear el archivo. Terminando proceso %d", hilo->proceso->pid);
        if (hilo->proceso == process_to_execute)
        {
            enviar_syscall_bloqueante(0, hilo->proceso->pid, END_BY_PROCESS_EXIT);
            log_info(kernel_logger, "## Finaliza el proceso %d", hilo->proceso->pid);
            signal_mutex(&proceso_en_ejecucion_mutex);
            return NULL;
        } 
        signal_mutex(&proceso_en_ejecucion_mutex);
        log_info(kernel_logger, "## Finaliza el proceso %d", hilo->proceso->pid);
        finalizacion_procesos(hilo->proceso);
    }
    free(argumentos);
    return NULL;
}

void DUMP_MEMORY_KERNEL(){
    wait_mutex(&proceso_en_ejecucion_mutex);
    int pid = process_to_execute->pid;
    signal_mutex(&proceso_en_ejecucion_mutex);
    wait_mutex(&mutex_hilo_en_ejecucion);
    tcb* hilo = hilo_en_ejecucion;
    int tid = hilo->tid;
    int fd_conexion_memoria = iniciar_conexion_memoria();
    // Enviar solicitud de dump a la memoria
    DUMP_args* argumentos = malloc(sizeof(DUMP_args));
    argumentos->hilo = hilo;
    argumentos->fd_conexion_memoria = fd_conexion_memoria;
    list_add_blocked(hilo);
    //log_debug(kernel_logger, "## (%d:%d) - bloqueado por DUMP", hilo->proceso->pid, hilo->tid);
    enviar_syscall_bloqueante(tid, pid, BLOCK_BY_DUMP);
    enviar_solicitud_dump(pid, tid, fd_conexion_memoria);
    signal_mutex(&mutex_hilo_en_ejecucion);
    // Esperar confirmación de la memoria
    pthread_t hilo_dump;
    pthread_create(&hilo_dump, NULL, dump, (void*)argumentos);
    pthread_detach(hilo_dump);
}

// Realiza una operación de I/O
void* esperar_IO(void* args) {
    IO_args* argumentos = ((IO_args*)args);
    tcb* hilo = argumentos->hilo;
    int milisegundos = argumentos->milisegundos;
    log_debug(kernel_logger, "## (%d:%d) - Inició IO", hilo->proceso->pid, hilo->tid);
    usleep(milisegundos * 1000); // Porque duerme en segundos
    list_remove_blocked(hilo);
    push_queue_ready(hilo);
    log_info(kernel_logger, "## (%d:%d) finalizó IO y pasa a READY", hilo->proceso->pid, hilo->tid);
    free(argumentos); // Liberar la memoria asignada para los argumentos
    return NULL;
}

void IO_KERNEL(int milisegundos) {
    wait_mutex(&mutex_hilo_en_ejecucion);
    tcb* hilo = hilo_en_ejecucion;
    enviar_syscall_bloqueante(hilo->tid, hilo->proceso->pid, BLOCK_IO);
    list_add_blocked(hilo);
    log_info(kernel_logger, "## (%d:%d) bloqueado por IO", hilo->proceso->pid, hilo->tid);

    // Crear y asignar memoria para los argumentos del hilo
    IO_args* argumentos = malloc(sizeof(IO_args));
    argumentos->hilo = hilo;
    argumentos->milisegundos = milisegundos;

    pthread_t hilo_IO;
    pthread_create(&hilo_IO, NULL, esperar_IO, (void*)argumentos);
    pthread_detach(hilo_IO);
    signal_mutex(&mutex_hilo_en_ejecucion);
}

void enviar_syscall_bloqueante(int tid, int pid, op_code syscall){
    t_paquete *paquete = iniciar_paquete(syscall);
    paquete->buffer->size = sizeof(int) * 2;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(pid), sizeof(int));
    offset += sizeof(int);
    memcpy(paquete->buffer->stream + offset, &(tid), sizeof(int));

    enviar_paquete(paquete, fd_conexion_interrupt);
    eliminar_paquete(paquete);
}

// GESTION DE SYSCALLS

// Junte recibir y ejecutar para ahorrarme otro switch

void* gestionar_syscalls( void* args ) {
    while (1)
    {
        t_paquete* paquete = recibir_paquete_syscall(fd_conexion_dispatch);
        //log_debug(kernel_logger, "Recibí syscall");
        wait_sem(&sem_en_syscall);
        //log_debug(kernel_logger, "Procesando syscall");
        int offset = 0;
        syscall_t syscall_tipo;
        memcpy(&syscall_tipo, paquete->buffer->stream + offset, sizeof(syscall_t));
        offset += sizeof(syscall_t);
        syscall_args* argumentos_syscall;
        wait_mutex(&mutex_hilo_en_ejecucion);
        wait_mutex(&proceso_en_ejecucion_mutex);
        log_info(kernel_logger, "## (%d:%d) - Solicitó syscall: %s", hilo_en_ejecucion->proceso->pid, hilo_en_ejecucion->tid, syscall_to_string(syscall_tipo));
        signal_mutex(&mutex_hilo_en_ejecucion);
        signal_mutex(&proceso_en_ejecucion_mutex);
        switch(syscall_tipo){
            case S_PROCESS_CREATE:
                argumentos_syscall = recibir_PROCESS_CREATE(paquete, offset);
                PROCESS_CREATE_KERNEL(argumentos_syscall->pseudocodigo, argumentos_syscall->tam_memoria, argumentos_syscall->prioridad_hilo);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            case S_PROCESS_EXIT:
                PROCESS_EXIT_KERNEL();
                enviar_confirmacion_syscall();
                break;
            case S_THREAD_CREATE:
                argumentos_syscall = recibir_THREAD_CREATE(paquete, offset);
                THREAD_CREATE_KERNEL(argumentos_syscall->pseudocodigo, argumentos_syscall->prioridad_hilo);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            case S_THREAD_JOIN:
                argumentos_syscall = recibir_TID_ARG(paquete, offset);
                THREAD_JOIN_KERNEL(argumentos_syscall->tid);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            case S_THREAD_CANCEL:
                argumentos_syscall = recibir_TID_ARG(paquete, offset);
                THREAD_CANCEL_KERNEL(argumentos_syscall->tid);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            case S_THREAD_EXIT:
                THREAD_EXIT_KERNEL();
                enviar_confirmacion_syscall();
                break;
            case S_MUTEX_CREATE:
                argumentos_syscall = recibir_SYSCALL_MUTEX(paquete, offset);
                MUTEX_CREATE_KERNEL(argumentos_syscall->recurso_mutex);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            case S_MUTEX_LOCK:
                argumentos_syscall = recibir_SYSCALL_MUTEX(paquete, offset);
                MUTEX_LOCK_KERNEL(argumentos_syscall->recurso_mutex);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            case S_MUTEX_UNLOCK:
                argumentos_syscall = recibir_SYSCALL_MUTEX(paquete, offset);
                MUTEX_UNLOCK_KERNEL(argumentos_syscall->recurso_mutex);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            case S_DUMP_MEMORY:
                DUMP_MEMORY_KERNEL();
                enviar_confirmacion_syscall();
                break;
            case S_IO:
                argumentos_syscall = recibir_SYSCALL_IO(paquete, offset);
                IO_KERNEL(argumentos_syscall->milisegundos);
                enviar_confirmacion_syscall();
                free_syscall_args(argumentos_syscall);
                break;
            default:
                log_error(kernel_logger, "Syscall desde CPU no reconocida");
                break;
            }
            eliminar_paquete(paquete);
    }
}

void enviar_confirmacion_syscall(){
    enviar_entero(fd_conexion_dispatch, OK);
}

// RECEPCION DE SYSCALLS (lo paso a conexiones?)

t_paquete* recibir_paquete_syscall(int conexion){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    
    // Recibe el código de operación
    recv(conexion, &(paquete->codigo_operacion), sizeof(int), 0);
    
    // Recibe el tamaño del buffer
    recv(conexion, &(paquete->buffer->size), sizeof(int), 0);
    
    // Asigna memoria para el stream
    paquete->buffer->stream = malloc(paquete->buffer->size);
    
    // Recibe el stream de datos
    recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);
    
    return paquete;
}

syscall_args* recibir_PROCESS_CREATE(t_paquete* paquete, int offset){
    syscall_args* argumentos = inicializar_syscalls_args();
    memcpy(&argumentos->tam_memoria, paquete->buffer->stream + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&argumentos->prioridad_hilo, paquete->buffer->stream + offset, sizeof(int));
    offset += sizeof(int);

    int longitud_pseudocodigo = paquete->buffer->size - offset;
    argumentos->pseudocodigo = malloc(longitud_pseudocodigo + 1);
    memcpy(argumentos->pseudocodigo, paquete->buffer->stream + offset, longitud_pseudocodigo);
    argumentos->pseudocodigo[longitud_pseudocodigo] = '\0';

    return argumentos;
}

syscall_args* recibir_THREAD_CREATE(t_paquete* paquete, int offset){
    syscall_args* argumentos = inicializar_syscalls_args();
    memcpy(&argumentos->prioridad_hilo, paquete->buffer->stream + offset, sizeof(int));
    offset += sizeof(int);

    // Correctly allocate and copy pseudocodigo
    size_t pseudocodigo_size = paquete->buffer->size - offset;
    argumentos->pseudocodigo = malloc(pseudocodigo_size + 1);
    memcpy(argumentos->pseudocodigo, paquete->buffer->stream + offset, pseudocodigo_size);
    argumentos->pseudocodigo[pseudocodigo_size] = '\0';

    return argumentos;
}

syscall_args* recibir_SYSCALL_IO(t_paquete* paquete, int offset){
    syscall_args* argumentos = inicializar_syscalls_args();
    memcpy(&argumentos->milisegundos, paquete->buffer->stream + offset, sizeof(int));

    return argumentos;
}

syscall_args* recibir_SYSCALL_MUTEX(t_paquete* paquete, int offset) {
    syscall_args* argumentos = inicializar_syscalls_args();
    
    int longitud_recurso_mutex = paquete->buffer->size - offset;
    argumentos->recurso_mutex = malloc(longitud_recurso_mutex + 1);
    memcpy(argumentos->recurso_mutex, paquete->buffer->stream + offset, longitud_recurso_mutex);
    argumentos->recurso_mutex[longitud_recurso_mutex] = '\0';

    return argumentos;
}

syscall_args* recibir_TID_ARG(t_paquete* paquete, int offset) {
    syscall_args* argumentos = inicializar_syscalls_args();
    memcpy(&argumentos->tid, paquete->buffer->stream + offset, sizeof(int));

    return argumentos;
}

void free_syscall_args(syscall_args* args)
{
    if(args){
        if(args->pseudocodigo){
            free(args->pseudocodigo);
        }
        if(args->recurso_mutex){
            free(args->recurso_mutex);
        }
        free(args);
    }
}

syscall_args *inicializar_syscalls_args()
{
    syscall_args *args = malloc(sizeof(syscall_args));
    args->pseudocodigo = NULL;
    args->recurso_mutex = NULL;
    return args;
}