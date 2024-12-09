#ifndef MEMORIA_PROCESOS_H_
#define MEMORIA_PROCESOS_H_

#include <utils/utils.h>
#include <logger.h>
#include <config.h>
#include <memoria_administracion.h>
#include <hilos.h>

void crear_proceso(int socket, t_proceso_paquete *paquete_proceso);
void finalizar_proceso(int pid);

#endif /* MEMORIA_PROCESOS_H_ */