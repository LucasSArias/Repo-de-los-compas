#ifndef MEMORIA_ADMINISTRACION_H_
#define MEMORIA_ADMINISTRACION_H_

#include <utils/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <string.h>
#include <stdint.h>
#include <config.h>
#include <logger.h>
#include <comunicacion.h>

#define BYTES_PER_OPERATION 4

// ESTRUCTURAS DE MEMORIA
typedef struct particion
{
    size_t inicio;  // Posición de inicio de la partición (base)
    size_t tamanio; // Tamaño de la partición (limite)
    int libre;      // 1 si está libre, 0 si está ocupada
    int pid;        // ID del proceso que ocupa la partición, -1 si está libre
} t_particion_memoria;

extern t_particion_memoria *lista_particiones;
extern void *espacio_memoria;
extern pthread_mutex_t partition_mutex;

void inicializar_memoria();
void inicializar_memoria_particiones_dinamicas();
void inicializar_memoria_particiones_fijas();
bool asignar_memoria_dinamica(size_t tamanio, int pid);
bool asignar_memoria_fija(size_t tamanio, int pid);
void liberar_memoria(size_t inicio);
void liberar_memoria_por_pid(int pid);
void escribir_memoria(size_t direccion_fisica, void *datos);
void leer_memoria(size_t direccion_fisica, void *buffer);
void consolidar_particiones();
void *get_memory_pid_content(int pid);
size_t tamanio_particion_por_pid(int pid);
void poblar_contexto_hilo_con_particion(t_new_contexto_paquete *contexto_ejecucion, int pid);

t_particion_memoria *first_fit_partition(size_t tamanio);
t_particion_memoria *best_fit_partition(size_t tamanio);
t_particion_memoria *worst_fit_partition(size_t tamanio);
t_particion_memoria *choose_algorithm_partition(size_t tamanio);
t_particion_memoria *find_partition_by_pid(int pid);

void memory_dump(int pid, int tid, int socket_request);

void destroy_memoria();

void loguear_particiones();

#endif /* MEMORIA_ADMINISTRACION_H_ */