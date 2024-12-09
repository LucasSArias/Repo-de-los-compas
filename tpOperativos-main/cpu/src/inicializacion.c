#include "inicializacion.h"

t_cpu_config *cpu_config = NULL;

void create_cpu_config(t_config *config_file)
{
    cpu_config = malloc(sizeof(t_cpu_config));

    cpu_config->puerto_escucha_dispatch = string_duplicate(config_get_string_value(config_file, P_PUERTO_ESCUCHA_DISPATCH));
    cpu_config->puerto_escucha_interrupt = string_duplicate(config_get_string_value(config_file, P_PUERTO_ESCUCHA_INTERRUPT));
    cpu_config->ip_memoria = string_duplicate(config_get_string_value(config_file, P_IP_MEMORIA));
    cpu_config->puerto_memoria = string_duplicate(config_get_string_value(config_file, P_PUERTO_MEMORIA));    
    cpu_config->log_level = log_level_from_string(config_get_string_value(config_file, P_LOG_LEVEL));
    /*
    // Print each member
    printf("##############################################################\n");
    printf("Puerto Escucha Dispatch: %s\n", cpu_config->puerto_escucha_dispatch);
    printf("Puerto Escucha Interrupt: %s\n", cpu_config->puerto_escucha_interrupt);
    printf("IP Memoria: %s\n", cpu_config->ip_memoria);
    printf("Puerto Memoria: %s\n", cpu_config->puerto_memoria);
    printf("Log Level: %d | %s\n", cpu_config->log_level, log_level_as_string(cpu_config->log_level));
    printf("##############################################################\n");
    */
    config_destroy(config_file);
}

void load_config_from_file(char *path) 
{
    t_config *config_file = config_create(path);
    t_log *logger_error = iniciar_logger("cpu.log", CPU, LOG_LEVEL_ERROR);

    if(config_file == NULL){
        log_error(logger_error, ERROR_FILE_NOT_FOUND, path, CPU);
        exit(EXIT_FAILURE);
    }

    log_destroy(logger_error);
    create_cpu_config(config_file);
}


void iniciar_logger_cpu()
{
    cpu_logger = iniciar_logger("cpu.log", CPU, cpu_config->log_level);
}

void inicializar_cpu(char *path)
{
    load_config_from_file(path);

    iniciar_logger_cpu();

    iniciar_conexiones_cpu(); //VER FUNCION

}
