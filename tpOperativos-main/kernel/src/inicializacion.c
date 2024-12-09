#include "estructuras.h"
#include "inicializacion.h"

t_log* kernel_logger; 

pthread_mutex_t mutex_queue_new;
pthread_mutex_t mutex_queue_ready_threads;
pthread_mutex_t mutex_list_blocked_threads;
pthread_mutex_t mutex_exit_threads;
pthread_mutex_t mutex_cola_de_prioridad;
pthread_mutex_t mutex_hilo_en_ejecucion;
pthread_mutex_t hilos_bloqueados_thread_join_mtx;
pthread_mutex_t creando_colas_por_prioridad;

int fd_conexion_dispatch;
int fd_conexion_interrupt;

t_queue*   queue_new_processes;
t_queue*   queue_ready_threads; 
t_list*   list_blocked_threads; 
t_list*            mutex_lista;
t_queue*   queue_exit_threads; 

sem_t hilo_interrumpido;
sem_t sem_CPU_libre_para_hilo;
sem_t espacio_para_proceso;
sem_t sem_en_syscall;
sem_t bloqueo_por_dump;
sem_t nuevos_hilos;
sem_t proceso_nuevo;
sem_t nuevos_hilos_para_cola;

// Inicializacion de loggers

void iniciar_logger_kernel() {
    kernel_logger = log_create("kernel.log", "[Kernel]", 1, LOG_LEVEL_TRACE);
    if(kernel_logger == NULL) {
        printf("No se pudo crear los logger de kernel\n");
        exit(EXIT_FAILURE);
    }
}

// Carga de configuracion en struct t_kernel_config

void crear_config_kernel(char* path_config) {
    t_config* config = config_create(path_config);
    if(config == NULL) {
        log_error(kernel_logger, "No se pudo crear el archivo de configuraciÃ³n");
        exit(EXIT_FAILURE);
    }

    kernel_config.IP_MEMORIA = strdup(config_get_string_value(config, "IP_MEMORIA"));
    kernel_config.PUERTO_MEMORIA = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
    kernel_config.IP_CPU = strdup(config_get_string_value(config, "IP_CPU"));
    kernel_config.PUERTO_CPU_DISPATCH = strdup(config_get_string_value(config, "PUERTO_CPU_DISPATCH"));
    kernel_config.PUERTO_CPU_INTERRUPT = strdup(config_get_string_value(config, "PUERTO_CPU_INTERRUPT"));
    kernel_config.ALGORITMO_PLANIFICACION = strdup(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    kernel_config.QUANTUM = config_get_int_value(config, "QUANTUM");

    config_destroy(config);
}

// Semaforos

void iniciar_mutex(){
    pthread_mutex_init(&mutex_queue_new, NULL);
    pthread_mutex_init(&mutex_queue_ready_threads, NULL);
    pthread_mutex_init(&mutex_list_blocked_threads, NULL);
    pthread_mutex_init(&mutex_exit_threads, NULL);
    pthread_mutex_init(&mutex_cola_de_prioridad, NULL);
    pthread_mutex_init(&mutex_hilo_en_ejecucion, NULL);
    pthread_mutex_init(&hilos_bloqueados_thread_join_mtx, NULL);
    pthread_mutex_init(&proceso_en_ejecucion_mutex, NULL);
    pthread_mutex_init(&creando_colas_por_prioridad, NULL);
}

void iniciar_semaforos(){
    sem_init(&hilo_interrumpido, 0, 0);
    sem_init(&sem_CPU_libre_para_hilo, 0, 1);
    sem_init(&espacio_para_proceso, 0, 1);
    sem_init(&sem_en_syscall, 0, 0);
    sem_init(&bloqueo_por_dump, 0, 0);
    sem_init(&nuevos_hilos, 0, 0);
    sem_init(&proceso_nuevo, 0, 0);
    sem_init(&nuevos_hilos_para_cola, 0, 0);
}

// Colas

void iniciar_colas(){
    queue_new_processes = queue_create();
    queue_ready_threads = queue_create();
    queue_exit_threads = queue_create();
}

void iniciar_listas() {
    list_blocked_threads = list_create();
    mutex_lista = list_create();
}

// Conexiones

void iniciar_conexion_cpu_dispatch(){
    fd_conexion_dispatch = crear_conexion(kernel_config.IP_CPU, kernel_config.PUERTO_CPU_DISPATCH);
    send_cliente_identificacion(fd_conexion_dispatch, HANDSHAKE_KERNEL);
    log_debug(kernel_logger, "Conexion con CPU Dispatch creada de forma exitosa.");
    
    op_code respuesta = recibir_entero(fd_conexion_dispatch);
    if(respuesta != HANDSHAKE_ACCEPTED)
    {
        log_error(kernel_logger, "No se pudo conectar con el servidor de DISPATCH");
        exit(EXIT_FAILURE);
    }
}

void iniciar_conexion_cpu_interrupt(){
    fd_conexion_interrupt = crear_conexion(kernel_config.IP_CPU, kernel_config.PUERTO_CPU_INTERRUPT);
    send_cliente_identificacion(fd_conexion_interrupt, HANDSHAKE_KERNEL);
    log_debug(kernel_logger, "Conexion con CPU Interrupt creada de forma exitosa.");
    
    op_code respuesta = recibir_entero(fd_conexion_interrupt);
    if(respuesta != HANDSHAKE_ACCEPTED)
    {
        log_error(kernel_logger, "No se pudo conectar con el servidor de INTERRUPT");
        exit(EXIT_FAILURE);
    }
}

//  Inicializacion del Kernel

void inicializar_kernel(char* path_config){
    iniciar_logger_kernel();
    crear_config_kernel(path_config);
    iniciar_conexion_cpu_dispatch();
    iniciar_conexion_cpu_interrupt();
    iniciar_mutex();
    iniciar_semaforos();
    iniciar_colas();
    iniciar_listas();
}
