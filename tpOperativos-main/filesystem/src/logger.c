#include <logger.h>

static const char *_decide_operacion(e_tipo_bloque condition);

void iniciar_logger_filesystem()
{
    filesystem_logger = iniciar_logger("filesystem.log", FILESYSTEM, filesystem_config->log_level);
}

void log_obligatorio_creacion_archivo(char *nombre_archivo, int tamanio)
{
    log_info(filesystem_logger, "## Archivo Creado: %s - Tamaño: %d", nombre_archivo, tamanio);
}

void log_obligatorio_asignacion_bloque(int nro_bloque, char *nombre_archivo, int bloques_libres)
{
    log_info(filesystem_logger, "## Bloque asignado: %d - Archivo: %s - Bloques Libres: %d",
             nro_bloque, nombre_archivo, bloques_libres);
}

void log_obligatorio_acceso_bloque(char *nombre_archivo, int nro_bloque_fs, e_tipo_bloque tipo_bloque)
{
    log_info(filesystem_logger, "## Acceso Bloque - Archivo: %s - Tipo Bloque: %s - Bloque File System %d",
             nombre_archivo, _decide_operacion(tipo_bloque), nro_bloque_fs);
}

void log_obligatorio_fin_peticion(char *nombre_archivo)
{
    log_info(filesystem_logger, "## Fin de solicitud - Archivo: %s", nombre_archivo);
}

static const char *_decide_operacion(e_tipo_bloque condition)
{
    switch (condition)
    {
    case DATOS:
        return "Datos";
    case INDICE:
        return "Índice";
    default:
        return "Desconocido";
    }
}