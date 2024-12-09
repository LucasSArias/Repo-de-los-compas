#include "config.h"

void load_config_from_file(char *path) 
{
    t_config *config_file = config_create(path);
    t_log *logger_error = iniciar_logger("filesystem.log", FILESYSTEM, LOG_LEVEL_ERROR);

    if(config_file == NULL){
        log_error(logger_error, ERROR_FILE_NOT_FOUND, path, FILESYSTEM);
        exit(EXIT_FAILURE);
    }

    log_destroy(logger_error);
    create_filesystem_config(config_file);
    config_destroy(config_file);
}

void destroy_config()
{
    free(filesystem_config->puerto_escucha);
    free(filesystem_config->mount_dir);
    free(filesystem_config);
}

struct stat st = {0};

void create_filesystem_config(t_config *config_file)
{
    filesystem_config = malloc(sizeof(t_filesystem_config));

    filesystem_config->block_count = config_get_int_value(config_file,P_BLOCK_COUNT);
    filesystem_config->block_size = config_get_int_value(config_file,P_BLOCK_SIZE);
    filesystem_config->log_level = log_level_from_string(config_get_string_value(config_file,P_LOG_LEVEL));
    filesystem_config->mount_dir = string_duplicate(config_get_string_value(config_file, P_MOUNT_DIR));
    filesystem_config->puerto_escucha = string_duplicate(config_get_string_value(config_file, P_PUERTO_ESCUCHA));
    filesystem_config->retardo_acceso_bloque = config_get_int_value(config_file, P_RETARDO_ACCESO_BLOQUE);

    /*
    // Print each member
    printf("##############################################################\n");
    printf("Puerto Escucha: %s\n", filesystem_config->puerto_escucha);
    printf("Mount Dir: %s\n", filesystem_config->mount_dir);
    printf("Block Size: %d\n", filesystem_config->block_size);
    printf("Block Count: %d\n", filesystem_config->block_count);
    printf("Retardo de respuesta: %d\n", filesystem_config->retardo_acceso_bloque);
    printf("Log Level: %d | %s\n", filesystem_config->log_level, log_level_as_string(filesystem_config->log_level));
    printf("##############################################################\n");
    */

    char *directory = string_from_format("%s", filesystem_config->mount_dir);
    if (stat(directory, &st) == -1)
    {
        mkdir(directory, 0700);
    }

    char *directory2 = string_from_format("%s/files", directory);
    if (stat(directory2, &st) == -1)
    {
        mkdir(directory2, 0700);
    }

    free(directory);
    free(directory2);
}
