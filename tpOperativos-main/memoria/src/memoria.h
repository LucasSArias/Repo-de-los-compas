#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <utils/utils.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>

#include <comunicacion.h>
#include <config.h>
#include <logger.h>
#include <memoria_administracion.h>
#include <hilos.h>
#include <procesos.h>

static volatile bool is_cpu_connected = false;

t_memoria_config *memoria_config;
t_log *memoria_logger;
size_t cantidad_particiones;
pthread_mutex_t partition_mutex;
pthread_mutex_t hilos_mute;

// FUNCIONES
void iniciar_logger_memoria();
void iniciar_servidor_memoria();
void *manejar_cliente(void *arg);
void terminar_memoria();
void manejar_cliente_kernel(int socket);
void manejar_cliente_cpu(int socket);
void validate_cpu_connection(int socket_cliente);

#endif /* MEMORIA_H_ */