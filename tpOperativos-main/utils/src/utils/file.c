#include <utils/file.h>

t_list *leer_archivo_por_linea(char *file_path, t_log *logger)
{
    t_list *lineas = list_create();
    
    FILE *archivo = fopen(file_path, "r");
    if (archivo == NULL) {
        log_error(logger, "Error al abrir el archivo: %s", file_path);
        return lineas;
    }

      // Lista para almacenar cada línea
    char *linea = NULL;
    size_t longitud = 0;

    // Leer cada línea del archivo
    while (getline(&linea, &longitud, archivo) != -1) {
        // Eliminar el salto de línea al final de la línea, si existe
        linea[strcspn(linea, "\n")] = '\0';
        list_add(lineas, string_duplicate(linea));  // Duplicar y agregar la línea a la lista
    }

    // Liberar recursos

    free(linea);
    fclose(archivo);

    return lineas;
}
