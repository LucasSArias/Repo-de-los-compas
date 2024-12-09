#ifndef FILESYSTEM_BLOQUES_H_
#define FILESYSTEM_BLOQUES_H_

#include "utils/file.h"
#include <fcntl.h>
#include <errno.h>
#include "config.h"
#include "logger.h"
#include <bitmap.h>
#include "communication.h"

#define BLOQUES_FILE_NAME "bloques.dat"

extern void *bloques;
extern size_t tamanio_bloques_total;
extern pthread_mutex_t mutex_bloques;

void inicializar_bloques();
void liberar_bloques();

void set_bloque_contenido(uint32_t index, void *contenido, size_t size);
uint32_t *reservar_bloques(uint32_t tamanio, int cantidad_bloques, char *file_name);
void grabar_bloques(uint32_t *indices, t_write_file_request *file_request, int cantidad_bloques);
void grabar_bloque_indice(uint32_t *indices, char *file_name, int cantidad_bloques);
bool validar_cantidad_punteros_x_bloque(int cantidad_bloques);

void retardo_acceso_bloque();

#endif /* FILESYSTEM_BLOQUES_H_ */