#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

#include <config.h>
#include <inicializacion.h>
#include <logger.h>
#include <connection.h>
#include <bloques.h>
#include <metadata.h>

#include <utils/utils.h>

t_filesystem_config *filesystem_config;
t_log *filesystem_logger;

uint32_t index_block;
void *bloques;
size_t tamanio_bloques_total;
char *bitmap;
size_t tamanio_bitmap;
uint32_t bloques_ocupados;

pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_bloques;

#endif /* FILESYSTEM_H_ */