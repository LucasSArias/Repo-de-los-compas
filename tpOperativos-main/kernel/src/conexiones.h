#ifndef CONEXIONESH
#define CONEXIONESH

#include "bibliotecas.h"
#include "estructuras.h"

// Conexion con memoria
int iniciar_conexion_memoria();
void cerrar_conexion_memoria(int fd_conexion_memoria);

// Interaccion con memoria para planificacion
bool espacio_para_proceso_en_memoria(pcb* proceso);
void notificar_finalizacion_proceso_memoria_y_esperar_confirmacion(pcb* proceso);

// Interaccion con CPU para planificacion
void enviar_hilo_exec_CPU(tcb* hilo);
void recibir_tid_post_exec();
void informar_interrupcion_por_quantum_CPU(tcb* hilo);

// Interaccion con memoria para syscalls
void notificar_finalizacion_hilo_memoria(tcb* hilo);
bool recibir_confirmacion_dump(int fd_conexion);
void enviar_informacion_proceso(pcb* proceso, int fd_conexion_memoria);
void enviar_solicitud_dump(int pid, int tid, int fd_conexion_memoria);
void enviar_informacion_hilo(tcb* hilo, int fd_conexion_memoria);

// funciones para memoria
op_code recibir_confirmacion(int fd_conexion);

#endif /* CONEXIONES_H */