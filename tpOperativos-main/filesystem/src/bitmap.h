#ifndef FILESYSTEM_BITMAP_H_
#define FILESYSTEM_BITMAP_H_

#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include "config.h"
#include "logger.h"
#include "utils/file.h"

#define BITMAP_FILE_NAME "bitmap.dat"

extern char *bitmap;
extern size_t tamanio_bitmap;
extern pthread_mutex_t mutex_bitmap;

void inicializar_bitmap();
void liberar_bitmap();

bool is_bloque_free(uint32_t index);
void ocupar_bloque(uint32_t index);
bool cantidad_libre_suficiente(uint32_t cantidad);
bool evaluar_espacio(uint32_t tamanio_archivo, int cantidad_bloques);
uint32_t cantidad_bloques_libres();
uint32_t find_first_free_block();

#endif /* FILESYSTEM_BITMAP_H_ */