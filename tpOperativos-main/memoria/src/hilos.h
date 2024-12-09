#ifndef MEMORIA_HILOS_H_
#define MEMORIA_HILOS_H_

#include <commons/collections/list.h>
#include <utils/utils.h>
#include <logger.h>
#include <comunicacion.h>

typedef struct Hilo
{
    int pid;
    int tid; // Identificador del hilo
    int ax, bx, cx, dx, ex, fx, gx, hx; // Registros de CPU
    int pc;                             // program counter
    t_list *instrucciones;              // almacena las instrucciones
} t_hilo;

extern t_hilo *lista_hilos;
extern size_t cantidad_hilos;
extern pthread_mutex_t hilos_mutex;

char *obtener_instruccion_with_PC(int pid, int tid, int pc);
void crear_contexto_hilo(t_hilo_paquete *hilo_paquete);
void finalizar_hilo(int pid, int tid);
void finalizar_hilos_por_pid(int pid);
t_new_contexto_paquete *obtener_contexto_hilo(int pid, int tid);
void actualizar_contexto_hilo(t_upd_contexto_paquete *upd_contexto_paquete);
void instruccion_destroy(void *ptr);

#endif /* MEMORIA_HILOS_H_ */