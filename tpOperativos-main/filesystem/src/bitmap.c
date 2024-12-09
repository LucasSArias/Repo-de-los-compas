#include "bitmap.h"

#define ESTADO_BLOQUE_FREE 0
#define ESTADO_BLOQUE_OCUPADO 1

void inicializar_bitmap()
{
    tamanio_bitmap = filesystem_config->block_count + (8 - 1) / 8;

    char *path = string_from_format("%s%s%s", filesystem_config->mount_dir, SEPARATOR_PATH, BITMAP_FILE_NAME);
    //FILE *bitmap_file = fopen(path, "w");
    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1 && (errno == EACCES || errno == ENOENT))
    {
        log_error(filesystem_logger, "No se pudo abrir o crear el archivo %s.", BITMAP_FILE_NAME);
        exit(EXIT_FAILURE);
    } else
    {
        if (errno != ENOENT)
        {
            log_debug(filesystem_logger, "El archivo %s ya existe.", BITMAP_FILE_NAME);
        } else 
        {
            log_obligatorio_creacion_archivo(BITMAP_FILE_NAME, tamanio_bitmap);
        }
    }
    int result = ftruncate(fd, tamanio_bitmap);
    if (result == -1)
    {
        log_error(filesystem_logger, "No se pudo cambiar el tamaño del archivo de bitmap");
        exit(EXIT_FAILURE);
    }

    bitmap = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bitmap == MAP_FAILED)
    {
        log_error(filesystem_logger, "No se pudo asignar memoria para el bitmap");
        exit(EXIT_FAILURE);
    }
    //memset(bitmap, 0, tamanio_bitmap);
    
    //fclose(bitmap_file);
    close(fd);
    free(path);
}

void liberar_bitmap()
{
    free(bitmap);
}

void ocupar_bloque(uint32_t index)
{
    bitmap[index] = 1;
}

bool cantidad_libre_suficiente(uint32_t cantidad)
{
    pthread_mutex_lock(&mutex_bitmap);
    for (uint32_t i = 0; i < tamanio_bitmap; i++)
    {
        if (bitmap[i] == 0)
        {
            cantidad--;
        }

        if (cantidad == 0)
        {
            //significa que hay espacio suficiente
            pthread_mutex_unlock(&mutex_bitmap);
            return true;
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);
    return false;
}

bool is_bloque_free(uint32_t index)
{
    int bloque_estado = bitmap[index];
    return bloque_estado == 0;
}

bool evaluar_espacio(uint32_t tamanio_archivo, int cantidad_bloques)
{
    return cantidad_libre_suficiente(cantidad_bloques);
}

uint32_t cantidad_bloques_libres()
{
    uint32_t cantidad_bloques_libres = 0;
    pthread_mutex_lock(&mutex_bitmap);
    for (uint32_t i = 0; i < tamanio_bitmap; i++)
    {
        if (is_bloque_free(i))
        {
            cantidad_bloques_libres++;
        }
    }
    pthread_mutex_unlock(&mutex_bitmap);
    return cantidad_bloques_libres;
}

uint32_t find_first_free_block()
{
    for (uint32_t i = 0; i < filesystem_config->block_count; i++)
    {
        pthread_mutex_lock(&mutex_bitmap);
        if (is_bloque_free(i))
        {
            ocupar_bloque(i);
            pthread_mutex_unlock(&mutex_bitmap);
            return i;
        }
        pthread_mutex_unlock(&mutex_bitmap);
    }
    return -1;
}