#ifndef PLANIFICACIONH
#define PLANIFICACIONH

#include "bibliotecas.h"
#include "estructuras.h"

/* PLANIFICADORES */

void* planificador_largo_plazo(void* arg);
void finalizacion_procesos(pcb* proceso);

void terminar_hilos_de_proceso(pcb* proceso);
void remover_hilo(tcb* hilo);

/* ALGORITMOS DE PLANIFICACION */

// FIFO
void* planificar_fifo_hilos( void* args );

// Prioridades
void* planificar_prioridades_hilos( void* args );

// Colas multinivel
void* planificar_cmn_hilos( void* args );
// Round Robin
void planificar_RR(t_queue *queue);

/* FUNCIONES */

void poner_en_ejecucion(tcb* hilo);

// Colas de priodidad

void* crear_colas_por_prioridad(void* arg);
void agregar_a_cola_prioridad(tcb* hilo);
bool find_by_priority(priority_queue* colas_prioridad, int prioridad_hilo);

priority_queue* devolver_lista_maxima_prioridad();

bool exists_priority(int prioridad_hilo);

void add_thread_by_priority(tcb* hilo);
void create_priority_queue(int prioridad_hilo);

// Quantum
void* quantum_interrupt(void* arg);
void* interrumpir_por_fin_quantum(void* arg);

void cancelar_ejecucion(tcb* hilo);



#endif /* PLANIFICACION_H */