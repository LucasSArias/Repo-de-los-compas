#ifndef FILESYSTEM_LOGGER_H_
#define FILESYSTEM_LOGGER_H_

#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <utils/constants.h>
#include <utils/utils.h>
#include <config.h>

extern t_log* filesystem_logger;

typedef enum TipoBloque
{   
    DATOS, 
    INDICE
} e_tipo_bloque;

void iniciar_logger_filesystem();
void log_obligatorio_creacion_archivo(char *nombre_archivo, int tamanio);
void log_obligatorio_asignacion_bloque(int nro_bloque, char *nombre_archivo, int bloques_libres);
void log_obligatorio_acceso_bloque(char *nombre_archivo, int nro_bloque_fs, e_tipo_bloque tipo_bloque);
void log_obligatorio_fin_peticion(char *nombre_archivo);

#endif /* FILESYSTEM_LOGGER_H_ */