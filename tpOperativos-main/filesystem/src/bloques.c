#include "bloques.h"

void inicializar_bloques()
{
    tamanio_bloques_total = filesystem_config->block_count * filesystem_config->block_size;

    char *path = string_from_format("%s%s%s", filesystem_config->mount_dir, SEPARATOR_PATH, BLOQUES_FILE_NAME);
    //FILE *bloques_file = fopen(path, "w");
    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1 && (errno == EACCES || errno == ENOENT))
    {
        log_error(filesystem_logger, "No se pudo abrir o crear el archivo %s.", BLOQUES_FILE_NAME);
        exit(EXIT_FAILURE);
    } else
    {
        if (errno != ENOENT)
        {
            log_debug(filesystem_logger, "El archivo %s ya existe.", BLOQUES_FILE_NAME);
        } else 
        {
            log_obligatorio_creacion_archivo(BLOQUES_FILE_NAME, tamanio_bloques_total);
        }
    }
    int result = ftruncate(fd, tamanio_bloques_total);
    if (result == -1)
    {
        log_error(filesystem_logger, "No se pudo cambiar el tamaño del archivo de bloques");
        exit(EXIT_FAILURE);
    }

    bloques = mmap(NULL, tamanio_bloques_total, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bloques == MAP_FAILED)
    {
        log_error(filesystem_logger, "No se pudo crear el mapeo de bloques");
        exit(EXIT_FAILURE);
    }
    // memset(bloques, 0, tamanio_bloques_total);

    //fclose(bloques_file);
    close(fd);
    free(path);
}

void liberar_bloques()
{
    free(bloques);
}

void set_bloque_contenido(uint32_t index, void *contenido, size_t size)
{
    pthread_mutex_lock(&mutex_bloques);
    memcpy(bloques + index * filesystem_config->block_size, contenido, size);
    msync(bloques, tamanio_bloques_total, MS_SYNC);
    pthread_mutex_unlock(&mutex_bloques);
}

uint32_t *reservar_bloques(uint32_t tamanio, int cantidad_bloques, char *file_name)
{
    uint32_t *indices = malloc(cantidad_bloques * sizeof(uint32_t));
    for (uint32_t i = 0; i < cantidad_bloques; i++)
    {
        indices[i] = find_first_free_block();
        if (indices[i] == -1)
        {
            // deberia liberar el espacio asignado
            log_error(filesystem_logger, "No hay mas espacio disponible para la creacion del archivo.");
            return NULL;
        }
        log_obligatorio_asignacion_bloque(indices[i], file_name, cantidad_bloques_libres());
    }
    pthread_mutex_lock(&mutex_bitmap);
    msync(bitmap, tamanio_bitmap, MS_SYNC);
    pthread_mutex_unlock(&mutex_bitmap);
    return indices;
}

void grabar_bloques(uint32_t *indices, t_write_file_request *file_request, int cantidad_bloques)
{
    int offset = 0;
    int size_to_copy;
    for (uint32_t i = 1; i < cantidad_bloques; i++)
    {
        if (offset + filesystem_config->block_size > file_request->tamanio)
        {
            size_to_copy = file_request->tamanio - offset;
        } else
        {
            size_to_copy = filesystem_config->block_size;
        }
        void *contenido_copia = malloc(size_to_copy);
        memcpy(contenido_copia, file_request->content + offset, size_to_copy);
        set_bloque_contenido(indices[i], contenido_copia, size_to_copy);
        free(contenido_copia);
        offset += filesystem_config->block_size;
        log_obligatorio_acceso_bloque(file_request->filename, indices[i], DATOS);
        retardo_acceso_bloque();
    }
}

void grabar_bloque_indice(uint32_t *indices, char *file_name, int cantidad_bloques)
{
    //int offset = 0;
    pthread_mutex_lock(&mutex_bloques);
    /*
    for (uint32_t i = 1; i < cantidad_bloques; i++)
    {
        memcpy(bloques + indices[0] * filesystem_config->block_size + offset, &indices[i], sizeof(uint32_t));
        offset += sizeof(uint32_t);      
    }
    */
    memcpy(bloques + indices[0] * filesystem_config->block_size, indices + 1, (cantidad_bloques - 1) * sizeof(uint32_t));
    msync(bloques, tamanio_bloques_total, MS_SYNC);
    pthread_mutex_unlock(&mutex_bloques);
    log_obligatorio_acceso_bloque(file_name, indices[0], INDICE);
    retardo_acceso_bloque();
}

bool validar_cantidad_punteros_x_bloque(int cantidad_bloques)
{
    int cantidad_punteros_necesarios = cantidad_bloques - 1;
    int cantidad_punteros_x_bloque = filesystem_config->block_size / sizeof(uint32_t);
    return cantidad_punteros_x_bloque >= cantidad_punteros_necesarios;
}

void retardo_acceso_bloque()
{
    usleep(filesystem_config->retardo_acceso_bloque * 1000);
}