#ifndef CPU_H
#define CPU_H

#include "bibliotecas.h"
#include "inicializacion.h"
#include "conexiones.h"
#include "cpu_dispatch.h"
#include "cpu_interrupt.h"
#include "ejecucion_hilos.h"
#include "finalizacion.h"
#include "syscall.h"
#include "mmu.h"

int conexion_dispatch;
int conexion_interrupt;
int conexion_memoria;

pthread_t hilo_interrupt;

t_contexto_ejecucion* contexto_ejecucion;
t_hilo* hilo_ejecutar;
t_instruccion *instruccion;

const char *enum_names_registros[TIPOS_REGISTROS] = {"PC", "AX", "BX", "CX", "DX", "EX", "FX", "GX", "HX"};

#endif /* CPU_H */