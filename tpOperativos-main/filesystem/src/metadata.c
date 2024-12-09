#include "metadata.h"
/*
t_metadata *leer_metadata(char *file_name)
{
    char *path = string_from_format("%s/files/%s", filesystem_config->mount_dir, file_name);
    FILE *archivo = fopen(path, "r");
    if (archivo == NULL)
    {
        log_error(filesystem_logger, "No se pudo abrir el archivo de metadatos");
        exit(EXIT_FAILURE);
    }
    t_metadata *metadata = malloc(sizeof(t_metadata));

    char *linea = NULL;
    size_t longitud = 0;
    // Leo la primera linea, el tamanio
    getline(&linea, &longitud, archivo);
    linea[strcspn(linea, "\n")] = '\0';
    char **aux = string_split(linea, "=");
    metadata->tamanio = atoi(aux[1]);
    string_array_destroy(aux);
    free(linea);
    // Leo la segunda linea, el indice del bloque
    getline(&linea, &longitud, archivo);
    linea[strcspn(linea, "\n")] = '\0';
    aux = string_split(linea, "=");
    metadata->indice_bloque = atoi(aux[1]);
    string_array_destroy(aux);
    free(linea);

    free(path);
    fclose(archivo);
    return metadata;
}
*/
void escribir_metadata(char *file_name, t_metadata *metadata)
{
    char *path = string_from_format("%s/files/%s", filesystem_config->mount_dir, file_name);
    FILE *archivo = fopen(path, "w");
    if (archivo == NULL)
    {
        log_error(filesystem_logger, "No se pudo crear el archivo de metadatos");
        exit(EXIT_FAILURE);
    }
    // Se podria tener un formato mas amigable
    // Ejemplo: X;Y
    fprintf(archivo, "%s=%d\n", "SIZE", metadata->tamanio);
    fprintf(archivo, "%s=%d\n", "INDEX_BLOCK", metadata->indice_bloque);
    fclose(archivo);
    log_obligatorio_creacion_archivo(file_name, metadata->tamanio);
    free(path);
    free(metadata);
}
