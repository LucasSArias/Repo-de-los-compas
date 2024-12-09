#ifndef CPU_INTERRUPT_H_
#define CPU_INTERRUPT_H_

#include "bibliotecas.h"
#include "inicializacion.h"


extern bool hay_interrupcion;
extern bool hay_syscall_bloqueante;
extern bool hay_segmentation_fault;
extern t_interrupcion interrupcion_quantum;
extern t_interrupcion interrupcion_syscall;

void* atender_cpu_interrupt( void* args);
t_interrupcion recibir_interrupcion_kernel();
void confirmar_interrupcion_kernel(t_interrupcion interrupcion);

#endif /* CPU_INTERRUPT_H_ */