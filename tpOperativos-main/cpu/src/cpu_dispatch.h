#ifndef CPU_DISPATCH_H_
#define CPU_DISPATCH_H_

#include "bibliotecas.h"

#include "inicializacion.h"
#include "ejecucion_hilos.h"

extern t_contexto_ejecucion *contexto_ejecucion;
extern t_hilo *hilo_ejecutar;

void atender_cpu_dispatch();
void liberar_hilo_ejecutar();
void liberar_contexto_ejecucion();
void liberar_ejecucion();

#endif /* CPU_DISPATCH_H_ */