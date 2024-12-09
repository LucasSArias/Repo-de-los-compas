#ifndef FILESYSTEM_COMMUNICATION_H_
#define FILESYSTEM_COMMUNICATION_H_

#include <commons/log.h>
#include <utils/utils.h>
#include <config.h>
#include <logger.h>

typedef struct WriteFileRequest
{
    uint32_t tamanio;
    char *filename;
    void *content;
} t_write_file_request;

t_write_file_request *recibir_write_file_request(int socket_cliente);
void liberar_write_file_request(t_write_file_request *write_file_request);

#endif /* FILESYSTEM_COMMUNICATION_H_ */