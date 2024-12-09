#ifndef FILESYSTEM_CONNECTION_H_
#define FILESYSTEM_CONNECTION_H_

#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <utils/constants.h>
#include <utils/utils.h>
#include "config.h"
#include <logger.h>
#include <communication.h>
#include <bitmap.h>
#include <bloques.h>
#include <metadata.h>

void iniciar_servidor_filesystem();
void *manejar_cliente(void *socketCliente);
void manejar_cliente_memoria(int socket);

void creacion_de_archivo(t_write_file_request *file_request, int socket);

#endif /* FILESYSTEM_CONNECTION_H_ */