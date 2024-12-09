#ifndef MEMORIA_CONFIG_H_
#define MEMORIA_CONFIG_H_

#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <utils/constants.h>
#include <utils/utils.h>
#include <commons/string.h>

typedef enum Algoritmo
{   
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} e_algoritmo;

typedef enum Esquema {
    FIJAS,
    DINAMICAS
} e_esquema;

typedef struct MemoriaConfig
{
    char *puerto_escucha;
    char *ip_filesystem;
    char *puerto_filesystem;
    size_t tamanio_memoria;
    char *path_instrucciones;
    size_t retardo_respuesta;
    e_esquema esquema;
    e_algoritmo algoritmo_busqueda;
    size_t* particiones; // Solo para particiones fijas
    t_log_level log_level;
} t_memoria_config;

extern t_memoria_config *memoria_config;
extern size_t cantidad_particiones;

void load_config_from_file(char *path);
void destroy_config();
void create_memoria_config(t_config *config_file);
size_t *get_array_from_config(t_config *config_file, char *key);
t_config* crear_config_memoria();
e_esquema esquema_from_string(char *esquema);
e_algoritmo algoritmo_from_string(char *algoritmo);

#endif /* MEMORIA_CONFIG_H_ */