#include <hilos.h>

t_hilo *lista_hilos; // Lista de hilos con sus registros e instrucciones
size_t cantidad_hilos = 0;
pthread_mutex_t hilos_mutex;

// se podria tener un t_list de hilos, tal vez es mas facil
void crear_contexto_hilo(t_hilo_paquete *hilo_paquete)
{
    pthread_mutex_lock(&hilos_mutex);
    cantidad_hilos++;
    lista_hilos = realloc(lista_hilos, cantidad_hilos * sizeof(t_hilo));

    // Inicializar registros de CPU a 0
    lista_hilos[cantidad_hilos - 1].pid = hilo_paquete->pid;
    lista_hilos[cantidad_hilos - 1].tid = hilo_paquete->tid;
    lista_hilos[cantidad_hilos - 1].ax = 0;
    lista_hilos[cantidad_hilos - 1].bx = 0;
    lista_hilos[cantidad_hilos - 1].cx = 0;
    lista_hilos[cantidad_hilos - 1].dx = 0;
    lista_hilos[cantidad_hilos - 1].ex = 0;
    lista_hilos[cantidad_hilos - 1].fx = 0;
    lista_hilos[cantidad_hilos - 1].gx = 0;
    lista_hilos[cantidad_hilos - 1].hx = 0;
    lista_hilos[cantidad_hilos - 1].pc = 0;
    lista_hilos[cantidad_hilos - 1].instrucciones = hilo_paquete->instrucciones;
    pthread_mutex_unlock(&hilos_mutex);

    // list_destroy_and_destroy_elements(hilo_paquete->instrucciones, instruccion_destroy);
    free(hilo_paquete);
}

void finalizar_hilo(int pid, int tid)
{
    pthread_mutex_lock(&hilos_mutex);
    for (size_t i = 0; i < cantidad_hilos; i++)
    {
        if (lista_hilos[i].pid == pid && lista_hilos[i].tid == tid)
        {
            list_destroy_and_destroy_elements(lista_hilos[i].instrucciones, instruccion_destroy);

            // Reordeno los hilos restantes
            for (size_t j = i; j < cantidad_hilos - 1; j++)
            {
                lista_hilos[j] = lista_hilos[j + 1];
            }

            cantidad_hilos--;
            if (cantidad_hilos > 0)
            {
                lista_hilos = realloc(lista_hilos, cantidad_hilos * sizeof(t_hilo));
                pthread_mutex_unlock(&hilos_mutex);
                return;
            }
            else 
            {
                free(lista_hilos);
                lista_hilos = NULL;
                pthread_mutex_unlock(&hilos_mutex);
                return;
            }
        }
    }
    pthread_mutex_unlock(&hilos_mutex);
    log_error(memoria_logger, "Error: TID %d no encontrado.", tid);
}

void finalizar_hilos_por_pid(int pid)
{
    pthread_mutex_lock(&hilos_mutex);
    for (size_t i = 0; i < cantidad_hilos; i++)
    {
        if (lista_hilos[i].pid == pid)
        {
            list_destroy_and_destroy_elements(lista_hilos[i].instrucciones, instruccion_destroy);

            for (size_t j = i; j < cantidad_hilos - 1; j++)
            {
                lista_hilos[j] = lista_hilos[j + 1];
            }

            cantidad_hilos--;
            if (cantidad_hilos > 0)
            {
                lista_hilos = realloc(lista_hilos, cantidad_hilos * sizeof(t_hilo));
                if (lista_hilos == NULL || !lista_hilos)
                {
                    log_error(memoria_logger, "Error: No se pudo reasignar memoria para la lista de hilos. Ex by: finalizar_proceso");
                    exit(EXIT_FAILURE);
                }
            }
            else 
            {
                free(lista_hilos);
                lista_hilos = NULL;
                pthread_mutex_unlock(&hilos_mutex);
                return;
            }
        }
    }
    pthread_mutex_unlock(&hilos_mutex);
}

t_new_contexto_paquete *obtener_contexto_hilo(int pid, int tid)
{
    t_new_contexto_paquete *contexto_ejecucion = malloc(sizeof(t_new_contexto_paquete));
    pthread_mutex_lock(&hilos_mutex);
    for (size_t i = 0; i < cantidad_hilos; i++)
    {
        if (lista_hilos[i].pid == pid && lista_hilos[i].tid == tid)
        {
            contexto_ejecucion->ax = lista_hilos[i].ax;
            contexto_ejecucion->bx = lista_hilos[i].bx;
            contexto_ejecucion->cx = lista_hilos[i].cx;
            contexto_ejecucion->dx = lista_hilos[i].dx;
            contexto_ejecucion->ex = lista_hilos[i].ex;
            contexto_ejecucion->fx = lista_hilos[i].fx;
            contexto_ejecucion->gx = lista_hilos[i].gx;
            contexto_ejecucion->hx = lista_hilos[i].hx;
            contexto_ejecucion->pc = lista_hilos[i].pc;
            break;
        }
    }
    pthread_mutex_unlock(&hilos_mutex);
    return contexto_ejecucion;
}

void actualizar_contexto_hilo(t_upd_contexto_paquete *upd_contexto_paquete)
{
    pthread_mutex_lock(&hilos_mutex);
    for (size_t i = 0; i < cantidad_hilos; i++)
    {
        if (lista_hilos[i].pid == upd_contexto_paquete->pid &&
            lista_hilos[i].tid == upd_contexto_paquete->tid)
        {
            lista_hilos[i].ax = upd_contexto_paquete->ax;
            lista_hilos[i].bx = upd_contexto_paquete->bx;
            lista_hilos[i].cx = upd_contexto_paquete->cx;
            lista_hilos[i].dx = upd_contexto_paquete->dx;
            lista_hilos[i].ex = upd_contexto_paquete->ex;
            lista_hilos[i].fx = upd_contexto_paquete->fx;
            lista_hilos[i].gx = upd_contexto_paquete->gx;
            lista_hilos[i].hx = upd_contexto_paquete->hx;
            lista_hilos[i].pc = upd_contexto_paquete->pc;
            break;
        }
    }
    pthread_mutex_unlock(&hilos_mutex);
}

// se podria tener un t_list de hilos, tal vez es mas facil
char *obtener_instruccion_with_PC(int pid, int tid, int pc)
{
    pthread_mutex_lock(&hilos_mutex);
    for (size_t i = 0; i < cantidad_hilos; i++)
    {
        if (lista_hilos[i].pid == pid && lista_hilos[i].tid == tid)
        {
            if (lista_hilos[i].pc < lista_hilos[i].instrucciones->elements_count)
            {
                char *instruccion = list_get(lista_hilos[i].instrucciones, pc);
                pthread_mutex_unlock(&hilos_mutex);
                return instruccion;
            }
            else
            {
                pthread_mutex_unlock(&hilos_mutex);
                return "FIN"; // Indica que no hay más instrucciones
            }
        }
    }
    pthread_mutex_unlock(&hilos_mutex);
    return "ERROR: PID:TID no encontrado.";
}

void instruccion_destroy(void *ptr)
{
    char *instruccion = (char *)ptr;
    free(instruccion);
}
