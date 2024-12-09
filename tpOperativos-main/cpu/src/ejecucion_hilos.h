#ifndef CPU_EXECUTE_HILO_H_
#define CPU_EXECUTE_HILO_H_

#include "bibliotecas.h"

#include <utils/constants.h>
#include "inicializacion.h"
#include "cpu_dispatch.h"
#include "cpu_interrupt.h"
#include "conexiones.h"
#include "syscall.h"
#include "execute.h"

extern t_instruccion *instruccion;

void ejecutar_hilo();
void fetch();
void decode();
void execute();
bool check_interrupt();
void recibir_confirmacion_syscall();
t_interrupcion inicializar_segmentation_fault();

e_registro convertir_a_registro(char *codigo_registro);

#endif /* CPU_EXECUTE_HILO_H_ */