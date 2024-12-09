#ifndef KERNELH
#define KERNELH

#include "bibliotecas.h"

pthread_t hilo_gestionar_syscalls;
pthread_t hilo_planificador_largo_plazo;
pthread_t hilo_planificador_corto_plazo;
pthread_t hilo_colas_por_prioridad;
pthread_t hilo_recibir_interrupciones_confirmadas;

#endif /* KERNEL_H */