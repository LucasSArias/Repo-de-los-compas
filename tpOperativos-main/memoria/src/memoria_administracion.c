#include <memoria_administracion.h>

void *espacio_memoria;
t_particion_memoria *lista_particiones;

// static bool _direccion_fisica_valida(int pid, size_t direccion_fisica);

void inicializar_memoria()
{
    if (memoria_config->esquema == FIJAS)
    {
        inicializar_memoria_particiones_fijas();
    }
    else if (memoria_config->esquema == DINAMICAS)
    {
        inicializar_memoria_particiones_dinamicas();
    }
    else
    {
        log_error(memoria_logger, "Error en inicializacion de memoria: Esquema de memoria no válido. Esquema: %d", memoria_config->esquema);
        exit(1);
    }
}

void inicializar_memoria_particiones_dinamicas()
{
    espacio_memoria = malloc(memoria_config->tamanio_memoria);
    cantidad_particiones = 1; // Inicialmente, toda la memoria es una partición libre
    lista_particiones = malloc(cantidad_particiones * sizeof(t_particion_memoria));

    lista_particiones[0].inicio = 0;
    lista_particiones[0].tamanio = memoria_config->tamanio_memoria;
    lista_particiones[0].libre = 1;
    lista_particiones[0].pid = -1;
}

void inicializar_memoria_particiones_fijas()
{
    espacio_memoria = malloc(memoria_config->tamanio_memoria); // Memoria total
    lista_particiones = malloc(cantidad_particiones * sizeof(t_particion_memoria));

    size_t offset = 0;
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        lista_particiones[i].inicio = offset;
        lista_particiones[i].tamanio = memoria_config->particiones[i];
        lista_particiones[i].libre = 1; // Todas las particiones comienzan libres
        lista_particiones[i].pid = -1;

        offset += memoria_config->particiones[i];
    }

    // Verificamos si el tamaño total de las particiones no excede la memoria disponible
    if (offset > memoria_config->tamanio_memoria)
    {
        log_error(memoria_logger, "Error: El tamaño total de las particiones excede la memoria disponible. Tamaño total: %zu, Memoria disponible: %zu", offset, memoria_config->tamanio_memoria);
        exit(EXIT_FAILURE);
    }
}

bool asignar_memoria_dinamica(size_t tamanio, int pid)
{
    // sincronizado
    t_particion_memoria *particion_a_asignar = choose_algorithm_partition(tamanio);

    /*
    No hace falta comprobar si el tamaño de la partición es suficiente o si esta libre
    ya que el algoritmo de asignacion de particiones debe asegurarlo
    */
    if (particion_a_asignar != NULL)
    {
        size_t espacio_restante = particion_a_asignar->tamanio - tamanio;

        particion_a_asignar->libre = 0;
        particion_a_asignar->pid = pid;
        particion_a_asignar->tamanio = tamanio;
        // Limpiar el espacio de memoria asignado. Tal vez no vaya a ser necesario
        memset(espacio_memoria + particion_a_asignar->inicio, 0, particion_a_asignar->tamanio);
        log_debug(memoria_logger, "Particion de tamaño %zu asignada al proceso %d", particion_a_asignar->tamanio, pid);
        
        // Si sobra espacio, crear una nueva partición para el espacio restante
        if (espacio_restante > 0)
        {
            t_particion_memoria nueva_particion;
            nueva_particion.inicio = particion_a_asignar->inicio + tamanio;
            nueva_particion.tamanio = espacio_restante;
            nueva_particion.libre = 1;
            nueva_particion.pid = -1;

            cantidad_particiones++;
            lista_particiones = realloc(lista_particiones, cantidad_particiones * sizeof(t_particion_memoria));

            if (lista_particiones == NULL || !lista_particiones)
            {
                log_error(memoria_logger, "Error: No se pudo reasignar memoria para la lista de particiones.");
                exit(EXIT_FAILURE);
            }

            // al hacer un realloc, el puntero de la particion_a_asignar se pierde
            particion_a_asignar = find_partition_by_pid(pid);
            size_t current_index = particion_a_asignar - lista_particiones;
            // Inserto la particion inmediatamente despues de la que se asigno
            for (size_t i = cantidad_particiones - 1; i > (current_index) + 1; i--)
            {
                lista_particiones[i] = lista_particiones[i - 1];
            }
            lista_particiones[(current_index) + 1] = nueva_particion;
        }

        return true; // Asignación exitosa
    }

    return false; // No se encontró un hueco adecuado
}

bool asignar_memoria_fija(size_t tamanio, int pid)
{
    // sincronizado
    t_particion_memoria *particion_a_asignar = choose_algorithm_partition(tamanio);

    /*
    No hace falta comprobar si el tamaño de la partición es suficiente o si esta libre
    ya que el algoritmo de asignacion de particiones debe asegurarlo
    */
    if (particion_a_asignar != NULL)
    {
        particion_a_asignar->libre = 0;
        particion_a_asignar->pid = pid;
        // Limpiar el espacio de memoria asignado. Tal vez no vaya a ser necesario
        memset(espacio_memoria + particion_a_asignar->inicio, 0, particion_a_asignar->tamanio);
        log_debug(memoria_logger, "Particion de tamaño %zu asignada al proceso %d", particion_a_asignar->tamanio, pid);
        return true; // Asignación exitosa
    }

    return false;
}

void liberar_memoria(size_t inicio)
{
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].inicio == inicio && !lista_particiones[i].libre)
        {
            lista_particiones[i].libre = 1; // Marcar la partición como libre
            lista_particiones[i].pid = -1;
            break;
        }
    }
}

void liberar_memoria_por_pid(int pid)
{
    pthread_mutex_lock(&partition_mutex);
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].pid == pid && !lista_particiones[i].libre)
        {
            lista_particiones[i].libre = 1; // Marcar la partición como libre
            lista_particiones[i].pid = -1;

            if (memoria_config->esquema == DINAMICAS)
            {
                consolidar_particiones();
            }
            loguear_particiones();
            break;
        }
    }

    pthread_mutex_unlock(&partition_mutex);
}

void escribir_memoria(size_t direccion_fisica, void *datos)
{
    pthread_mutex_lock(&partition_mutex);
    /*
    // Verificar que la lectura no exceda los límites de la memoria
    bool valido = _direccion_fisica_valida(pid, direccion_fisica);
    if (!valido)
    {
        log_error(memoria_logger, "Error: Dirección fisica no válida para el PID: %d", pid);
        pthread_mutex_unlock(&partition_mutex);
        return;
    }
    */
    // Copiar los datos al espacio de memoria a partir de 'inicio'
    memcpy(espacio_memoria + direccion_fisica, datos, BYTES_PER_OPERATION);
    pthread_mutex_unlock(&partition_mutex);
}

void leer_memoria(size_t direccion_fisica, void *buffer)
{
    pthread_mutex_lock(&partition_mutex);
    /*
    // Verificar que la lectura no exceda los límites de la memoria
    bool valido = _direccion_fisica_valida(pid, direccion_fisica);
    if (!valido)
    {
        log_error(memoria_logger, "Error: Dirección fisica no válida para el PID: %d", pid);
        pthread_mutex_unlock(&partition_mutex);
        return;
    }
    */
    // Copiar los datos de la memoria al buffer proporcionado
    memcpy(buffer, (char *)espacio_memoria + direccion_fisica, BYTES_PER_OPERATION);
    pthread_mutex_unlock(&partition_mutex);
}

void consolidar_particiones()
{
    // sincronizado
    for (size_t i = 0; i < cantidad_particiones - 1; i++)
    {
        if (lista_particiones[i].libre && lista_particiones[i + 1].libre)
        {
            // Uno el tamanio de las dos particiones
            lista_particiones[i].tamanio += lista_particiones[i + 1].tamanio;

            // reduzco el array de particiones
            for (size_t j = i + 1; j < cantidad_particiones - 1; j++)
            {
                lista_particiones[j] = lista_particiones[j + 1];
            }

            cantidad_particiones--;
            lista_particiones = realloc(lista_particiones, cantidad_particiones * sizeof(t_particion_memoria));
            if (lista_particiones == NULL || !lista_particiones)
            {
                log_error(memoria_logger, "Error: No se pudo reasignar memoria para la lista de particiones. Ex by: consolidar_particiones");
                exit(1);
            }

            // Vuelve pa tras
            i--;
        }
    }
}

void *get_memory_pid_content(int pid)
{
    // sincronizado
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].pid == pid && !lista_particiones[i].libre)
        {
            void *contenido_copia = malloc(lista_particiones[i].tamanio);
            memcpy(contenido_copia, espacio_memoria + lista_particiones[i].inicio, lista_particiones[i].tamanio);
            return contenido_copia;
        }
    }
    return NULL;
}

size_t tamanio_particion_por_pid(int pid)
{
    pthread_mutex_lock(&partition_mutex);
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].pid == pid && !lista_particiones[i].libre)
        {
            pthread_mutex_unlock(&partition_mutex);
            return lista_particiones[i].tamanio;
        }
    }
    pthread_mutex_unlock(&partition_mutex);
    return 0;
}

void poblar_contexto_hilo_con_particion(t_new_contexto_paquete *contexto_ejecucion, int pid)
{
    pthread_mutex_lock(&partition_mutex);
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].pid == pid)
        {
            contexto_ejecucion->base = lista_particiones[i].inicio;
            contexto_ejecucion->limite = lista_particiones[i].inicio + lista_particiones[i].tamanio;
            break;
        }
    }
    pthread_mutex_unlock(&partition_mutex);
}

t_particion_memoria *first_fit_partition(size_t tamanio)
{
    // sincronizado
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].libre && lista_particiones[i].tamanio >= tamanio)
        {
            return &lista_particiones[i];
        }
    }
    return NULL;
}

t_particion_memoria *best_fit_partition(size_t tamanio)
{
    // sincronizado
    t_particion_memoria *mejor_particion = NULL;
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].libre && lista_particiones[i].tamanio >= tamanio)
        {
            if (mejor_particion == NULL || mejor_particion->tamanio > lista_particiones[i].tamanio)
            {
                mejor_particion = &lista_particiones[i];
            }
        }
    }
    return mejor_particion;
}

t_particion_memoria *worst_fit_partition(size_t tamanio)
{
    // sincronizado
    t_particion_memoria *peor_particion = NULL;
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].libre && lista_particiones[i].tamanio >= tamanio)
        {
            if (peor_particion == NULL || peor_particion->tamanio < lista_particiones[i].tamanio)
            {
                peor_particion = &lista_particiones[i];
            }
        }
    }
    return peor_particion;
}

t_particion_memoria *choose_algorithm_partition(size_t tamanio)
{
    // sincronizado
    switch (memoria_config->algoritmo_busqueda)
    {
    case FIRST_FIT:
        return first_fit_partition(tamanio);
    case BEST_FIT:
        return best_fit_partition(tamanio);
    case WORST_FIT:
        return worst_fit_partition(tamanio);
    default:
        log_error(memoria_logger, "Error: Algoritmo de busqueda no reconocido. Algoritmo: %d", memoria_config->algoritmo_busqueda);
        exit(1);
    }
}

/*
static bool _direccion_fisica_valida(int pid, size_t direccion_fisica)
{
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].pid == pid && !lista_particiones[i].libre)
        {
            // Se tiene que leer/escribir si o si la cantidad de bytes especificados en el TP, 4 bytes
            if (lista_particiones[i].inicio <= direccion_fisica &&
                lista_particiones[i].inicio + lista_particiones[i].tamanio > direccion_fisica + BYTES_PER_OPERATION)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}
*/

t_particion_memoria *find_partition_by_pid(int pid)
{
    for (size_t i = 0; i < cantidad_particiones; i++)
    {
        if (lista_particiones[i].pid == pid && !lista_particiones[i].libre)
        {
            return &lista_particiones[i];
        }
    }
    return NULL;
}

void memory_dump(int pid, int tid, int socket_request)
{
    /*
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);
    int miliseconds = tv.tv_usec / 1000;
    char *timestamp = string_from_format("%02d:%02d:%02d:%03d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, miliseconds);
    char *filename = string_from_format("%d-%d-%s.dmp", pid, tid, timestamp);
    free(timestamp);
    //free(tm_info);
    */
    char *timestamp = temporal_get_string_time("%H:%M:%S:%MS");
    char *filename = string_from_format("%d-%d-%s.dmp", pid, tid, timestamp);
    free(timestamp);
    pthread_mutex_lock(&partition_mutex);
    void *memory_content = get_memory_pid_content(pid);
    pthread_mutex_unlock(&partition_mutex);
    if (memory_content == NULL)
    {
        // char *content = string_from_format("Error: No se pudo obtener el contenido de la memoria para el PID: %d", pid);
        // enviar_respuesta_con_mensaje(socket_request, ERROR, content);
        enviar_estado_solicitud(socket_request, ERROR);
        // free(content);
        free(filename);
        return;
    }
    int size_content = tamanio_particion_por_pid(pid);
    int conexion_filesystem = conectar_con_filesystem();
    enviar_solicitud_memory_dump(filename, memory_content, conexion_filesystem, size_content);
    //char *respuesta = recibir_respuesta_de_filesystem(conexion_filesystem);
    op_code status_op = recibir_entero(conexion_filesystem);
    /*
    if (respuesta != NULL)
    {
        status_op = ERROR;
        log_warning(memoria_logger, "No se pudo obtener el contenido de la memoria para el PID: %s", respuesta);
        free(respuesta);
    }
    else
    {
        status_op = OK;
    }
    */
    // enviar_respuesta_con_mensaje(socket_request, status_op, respuesta);
    if (status_op == ERROR)
    {
        log_warning(memoria_logger, "No se pudo crear el archivo para el PID %d", pid);
    }
    enviar_estado_solicitud(socket_request, status_op);

    free(filename);
    free(memory_content);
}

void destroy_memoria()
{
    free(espacio_memoria);
    free(lista_particiones);
}

void loguear_particiones()
{
   char *str = string_duplicate("{");
   for (size_t i = 0; i < cantidad_particiones; i++)
   {
       string_append_with_format(&str, "[%zu;%d]", lista_particiones[i].tamanio, lista_particiones[i].pid);
   }
   string_append(&str, "}");
   log_debug(memoria_logger, "%s", str);
   free(str);
}