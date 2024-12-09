#include <config.h>

#define ESQUEMA_ENUM_SIZE 2
static char *enum_names_esquema[ESQUEMA_ENUM_SIZE] = {"FIJAS", "DINAMICAS"};

#define ALGORITMO_ENUM_SIZE 3
static char *enum_names_algoritmo[ALGORITMO_ENUM_SIZE] = {"FIRST", "BEST", "WORST"};

void load_config_from_file(char *path) 
{
    t_config *config_file = config_create(path);
    t_log *logger_error = iniciar_logger("memoria.log", MEMORIA, LOG_LEVEL_ERROR);

    if(config_file == NULL){
        log_error(logger_error, ERROR_FILE_NOT_FOUND, path, MEMORIA);
        exit(EXIT_FAILURE);
    }

    log_destroy(logger_error);
    create_memoria_config(config_file);
}

void destroy_config()
{
    free(memoria_config->puerto_escucha);
    free(memoria_config->ip_filesystem);
    free(memoria_config->puerto_filesystem);
    free(memoria_config->particiones);
    free(memoria_config->path_instrucciones);
    free(memoria_config);
}

void create_memoria_config(t_config *config_file)
{
    memoria_config = malloc(sizeof(t_memoria_config));

    memoria_config->puerto_escucha = string_duplicate(config_get_string_value(config_file,P_PUERTO_ESCUCHA));
    memoria_config->ip_filesystem = string_duplicate(config_get_string_value(config_file,P_IP_FILESYSTEM));
    memoria_config->puerto_filesystem = string_duplicate(config_get_string_value(config_file,P_PUERTO_FILESYSTEM));
    memoria_config->algoritmo_busqueda = algoritmo_from_string(config_get_string_value(config_file, P_ALGORITMO_BUSQUEDA));
    memoria_config->esquema = esquema_from_string(config_get_string_value(config_file, P_ESQUEMA));
    memoria_config->particiones = get_array_from_config(config_file, P_PARTICIONES);
    memoria_config->tamanio_memoria = config_get_int_value(config_file, P_TAM_MEMORIA);
    memoria_config->log_level = log_level_from_string(config_get_string_value(config_file,P_LOG_LEVEL));
    memoria_config->path_instrucciones = string_duplicate(config_get_string_value(config_file, P_PATH_INSTRUCCIONES));
    memoria_config->retardo_respuesta = config_get_int_value(config_file, P_RETARDO_RESPUESTA);
    /*
    // Print each member
    printf("##############################################################\n");
    printf("Puerto Escucha: %s\n", memoria_config->puerto_escucha);
    printf("IP Filesystem: %s\n", memoria_config->ip_filesystem);
    printf("Puerto Filesystem: %s\n", memoria_config->puerto_filesystem);
    printf("Algoritmo de Busqueda: %d | %s\n", memoria_config->algoritmo_busqueda, enum_names_algoritmo[memoria_config->algoritmo_busqueda]);
    printf("Esquema: %d | %s\n", memoria_config->esquema, enum_names_esquema[memoria_config->esquema]);
    printf("Tamaño de Memoria: %zu\n", memoria_config->tamanio_memoria);
    printf("Log Level: %d | %s\n", memoria_config->log_level, log_level_as_string(memoria_config->log_level));
    printf("Ruta de instrucciones: %s\n", memoria_config->path_instrucciones);
    printf("Retardo de respuesta: %zu\n", memoria_config->retardo_respuesta);

    // Print each partition size
    if (memoria_config->particiones != NULL) {
        for (size_t i = 0; i < cantidad_particiones; i++) {
            printf("Particion %zu: %zu\n", i, memoria_config->particiones[i]);
        }
    } else {
        printf("No Particiones Configuradas\n");
    }
    printf("##############################################################\n");
    */
    config_destroy(config_file);
}

size_t *get_array_from_config(t_config *config_file, char *key)
{
    char **array = NULL;
    // valido que el array este bien declarado, []. Si el esquema es dinamica, no lo necesitamos.
    if(config_has_property(config_file, key)) {
        char *str_in_file = config_get_string_value(config_file, key);
        if(str_in_file != NULL && strlen(str_in_file) > 0)
            array = config_get_array_value(config_file, key);
    } 
    if(array == NULL){
        return NULL;
    }
    size_t *size_array = NULL;
    cantidad_particiones = 0;
    for (int i = 0; array[i] != NULL; i++)
    {
        if(size_array == NULL){
            size_array = malloc(sizeof(size_t));
        } else {
            size_array = realloc(size_array, sizeof(size_t) * (i + 1));
        }
        size_array[i] = atoi(array[i]);
        cantidad_particiones++;
    }
    string_array_destroy(array);
    return size_array;
}

t_config* crear_config_memoria() 
{
    t_config* config = config_create("memoria.config");
    if (config == NULL) {
        //log_error(memoria_logger, "No se pudo abrir el archivo de configuracion");
        exit(1);
    }
    return config;
}

e_esquema esquema_from_string(char *esquema) {
	for (int i = 0; i < ESQUEMA_ENUM_SIZE; i++) {
		if (string_equals_ignore_case(esquema, enum_names_esquema[i])){
			return i;
		}
	}

	return -1;
}

e_algoritmo algoritmo_from_string(char *algoritmo) {
	for (int i = 0; i < ALGORITMO_ENUM_SIZE; i++) {
		if (string_equals_ignore_case(algoritmo, enum_names_algoritmo[i])){
			return i;
		}
	}

	return -1;
}