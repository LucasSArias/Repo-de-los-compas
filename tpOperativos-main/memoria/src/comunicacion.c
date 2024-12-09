#include <comunicacion.h>

t_hilo_paquete *recibir_hilo(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    t_hilo_paquete *hilo_paquete = malloc(sizeof(t_hilo_paquete));
    int offset = 0;
    // pid
    memcpy(&(hilo_paquete->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    // tid
    memcpy(&(hilo_paquete->tid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    // instrucciones
    // Extraer el path del pseudocódigo
    int pseudocodigo_len = buffer->size - offset;
    char *path_pseudocodigo = malloc(pseudocodigo_len + 1);
    memcpy(path_pseudocodigo, buffer->stream + offset, pseudocodigo_len);
    path_pseudocodigo[pseudocodigo_len] = '\0';
    
    //Leo el path y el archivo, guardando el contenido en la lista de instrucciones
    char *absolute_path = string_from_format("%s%s%s", memoria_config->path_instrucciones, SEPARATOR_PATH, path_pseudocodigo);
    log_debug(memoria_logger, "Leyendo instrucciones de: %s", absolute_path);
    hilo_paquete->instrucciones = leer_archivo_por_linea(absolute_path, memoria_logger);

    // Liberar memoria temporal
    eliminar_buffer(buffer);
    free(path_pseudocodigo);
    free(absolute_path);
    return hilo_paquete;
}

t_proceso_paquete *recibir_proceso(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Asignar memoria para el stream con el tamaño recibido
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Deserializar el contenido del stream en un t_proceso_paquete
    t_proceso_paquete *proceso_paquete = malloc(sizeof(t_proceso_paquete));
    int offset = 0;

    // Extraer el pid
    memcpy(&(proceso_paquete->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Extraer el tamaño
    memcpy(&(proceso_paquete->tamanio), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // instrucciones
    // Extraer el path del pseudocódigo
    int pseudocodigo_len = buffer->size - offset;
    char *path_pseudocodigo = malloc(pseudocodigo_len + 1);
    memcpy(path_pseudocodigo, buffer->stream + offset, pseudocodigo_len);
    path_pseudocodigo[pseudocodigo_len] = '\0';
    
    //Leo el path y el archivo, guardando el contenido en la lista de instrucciones
    char *absolute_path = string_from_format("%s%s%s", memoria_config->path_instrucciones, SEPARATOR_PATH, path_pseudocodigo);
    log_debug(memoria_logger, "Leyendo instrucciones de: %s", absolute_path);
    proceso_paquete->instrucciones = leer_archivo_por_linea(absolute_path, memoria_logger);

    // Liberar memoria del buffer
    eliminar_buffer(buffer);
    free(path_pseudocodigo);
    free(absolute_path);
    return proceso_paquete;
}

int recibir_id(int socket_cliente)
{
    int id;
    int size;
    recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
    recv(socket_cliente, &id, size, MSG_WAITALL);
    return id;
}

t_upd_contexto_paquete *recibir_upd_contexto(int socket_cliente)
{
    // Inicializar buffer
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Asignar memoria para el stream según el tamaño recibido
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Deserializar el contenido del stream en un t_upd_contexto_paquete
    t_upd_contexto_paquete *contexto_paquete = malloc(sizeof(t_upd_contexto_paquete));
    int offset = 0;

    // Extraer el pid
    memcpy(&(contexto_paquete->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Extraer el tid
    memcpy(&(contexto_paquete->tid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Extraer los registros de CPU (AX, BX, CX, DX, EX, FX, GX, HX)
    memcpy(&(contexto_paquete->ax), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(contexto_paquete->bx), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(contexto_paquete->cx), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(contexto_paquete->dx), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(contexto_paquete->ex), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(contexto_paquete->fx), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(contexto_paquete->gx), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(contexto_paquete->hx), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Extraer el valor de PC
    memcpy(&(contexto_paquete->pc), buffer->stream + offset, sizeof(uint32_t));

    // Liberar memoria del buffer
    eliminar_buffer(buffer);

    // Devolver el paquete de contexto
    return contexto_paquete;
}

t_memory_dump_request *recibir_memory_dump_request(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Reservar espacio para el contenido del buffer y recibir el stream completo
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Reservar espacio para el struct t_memory_dump_request
    t_memory_dump_request *memory_dump_request = malloc(sizeof(t_memory_dump_request));

    // Deserializar los datos desde el stream del buffer
    int offset = 0;
    memcpy(&(memory_dump_request->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(memory_dump_request->tid), buffer->stream + offset, sizeof(uint32_t));

    // Liberar memoria del buffer
    eliminar_buffer(buffer);

    // Devolver el paquete de solicitud de volcado de memoria
    return memory_dump_request;
}

char *recibir_respuesta_de_filesystem(int conexion_filesystem)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));

    recv(conexion_filesystem, &(paquete->codigo_operacion), sizeof(op_code), MSG_WAITALL);
    recv(conexion_filesystem, &(paquete->buffer->size), sizeof(int), MSG_WAITALL);

    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(conexion_filesystem, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

    if (paquete->codigo_operacion == OK)
    {
        eliminar_paquete(paquete);
        return NULL;
    }

    // Copiar el contenido a una nueva variable que se devolverá
    char *content = malloc(paquete->buffer->size + 1); // +1 para el terminador de string

    memcpy(content, paquete->buffer->stream, paquete->buffer->size);
    content[paquete->buffer->size] = '\0'; // Agregar el terminador de string

    // Liberar memoria del buffer
    eliminar_paquete(paquete);
    close(conexion_filesystem);

    return content;
}

t_memory_read_request *recibir_memory_read_request(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Reservar espacio para el contenido del buffer y recibir el stream completo
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Reservar espacio para el struct t_memory_read_request
    t_memory_read_request *memory_access_request = malloc(sizeof(t_memory_read_request));

    // Deserializar los datos desde el stream del buffer
    int offset = 0;
    memcpy(&(memory_access_request->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(memory_access_request->tid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(memory_access_request->direccion_fisica), buffer->stream + offset, sizeof(uint32_t));

    // Liberar memoria del buffer
    eliminar_buffer(buffer);

    // Devolver el paquete de solicitud de acceso a memoria
    return memory_access_request;
}

t_memory_write_request *recibir_memory_write_request(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Reservar espacio para el contenido del buffer y recibir el stream completo
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Reservar espacio para el struct t_memory_write_request
    t_memory_write_request *memory_access_request = malloc(sizeof(t_memory_write_request));

    // Deserializar los datos desde el stream del buffer
    int offset = 0;
    memcpy(&(memory_access_request->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(memory_access_request->tid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(memory_access_request->direccion_fisica), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    memory_access_request->buffer = malloc(BYTES_PER_OPERATION);
    memcpy(memory_access_request->buffer, buffer->stream + offset, BYTES_PER_OPERATION);

    // Liberar memoria del buffer
    eliminar_buffer(buffer);

    // Devolver el paquete de solicitud de acceso a memoria
    return memory_access_request;
}

t_get_contexto_ejecucion_request *recibir_get_contexto_ejecucion_request(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Reservar espacio para el contenido del buffer y recibir el stream completo
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Reservar espacio para el struct t_get_contexto_ejecucion_request
    t_get_contexto_ejecucion_request *get_contexto_ejecucion_request = malloc(sizeof(t_get_contexto_ejecucion_request));

    // Deserializar los datos desde el stream del buffer
    int offset = 0;
    memcpy(&(get_contexto_ejecucion_request->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(get_contexto_ejecucion_request->tid), buffer->stream + offset, sizeof(uint32_t));

    // Liberar memoria del buffer
    eliminar_buffer(buffer);

    return get_contexto_ejecucion_request;
}

t_get_instruccion_request *recibir_get_instruccion_request(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Reservar espacio para el contenido del buffer y recibir el stream completo
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Reservar espacio para el struct t_get_instruccion_request
    t_get_instruccion_request *get_instruccion_request = malloc(sizeof(t_get_instruccion_request));

    // Deserializar los datos desde el stream del buffer
    int offset = 0;
    memcpy(&(get_instruccion_request->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(get_instruccion_request->tid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(get_instruccion_request->pc), buffer->stream + offset, sizeof(uint32_t));

    // Liberar memoria del buffer
    eliminar_buffer(buffer);

    return get_instruccion_request;
}

t_thread_to_end_request *recibir_thread_to_end(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño del buffer
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);

    // Reservar espacio para el contenido del buffer y recibir el stream completo
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Reservar espacio para el struct t_thread_to_end_request
    t_thread_to_end_request *thread_to_end_request = malloc(sizeof(t_thread_to_end_request));

    // Deserializar los datos desde el stream del buffer
    int offset = 0;
    memcpy(&(thread_to_end_request->pid), buffer->stream + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&(thread_to_end_request->tid), buffer->stream + offset, sizeof(uint32_t));

    // Liberar memoria del buffer
    eliminar_buffer(buffer);

    // Devolver el paquete de solicitud de acceso a memoria
    return thread_to_end_request;
}

void enviar_estado_solicitud(int socket_cliente, op_code codigo_operacion)
{
    enviar_entero(socket_cliente, codigo_operacion);
}

void enviar_solicitud_memory_dump(char *filename, void *content, int conexion_filesystem, size_t size_content)
{
    t_paquete *paquete = iniciar_paquete(WRITE_FILE);
    paquete->buffer->size = strlen(filename) + size_content + sizeof(size_t) * 2;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    
    size_t len_filename = strlen(filename);
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(len_filename), sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(paquete->buffer->stream + offset, filename, len_filename);
    offset += len_filename;
    memcpy(paquete->buffer->stream + offset, &(size_content), sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(paquete->buffer->stream + offset, content, size_content);

    enviar_paquete(paquete, conexion_filesystem);
    eliminar_paquete(paquete);
}

void enviar_respuesta_lectura_memoria_OK(int socket_cliente, void *buffer)
{
    t_paquete *paquete = iniciar_paquete(OK);
    paquete->buffer->size = BYTES_PER_OPERATION;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, buffer, paquete->buffer->size);

    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
    free(buffer);
}

void enviar_instruccion_a_cpu(char *instruccion, int socket_cliente)
{
    t_paquete *paquete = iniciar_paquete(OK);
    paquete->buffer->size = strlen(instruccion);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, instruccion, paquete->buffer->size);

    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
}

void sleep_milliseconds(unsigned int milliseconds)
{
    usleep(milliseconds * 1000);
}

void enviar_contexto_hilo(int socket_cliente, op_code codigo_operacion, t_new_contexto_paquete *contexto_paquete)
{
    t_paquete *paquete = iniciar_paquete(codigo_operacion);
    paquete->buffer->size = sizeof(t_new_contexto_paquete);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    
    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->ax), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->bx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->cx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->dx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->ex), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->fx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->gx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->hx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->pc), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->base), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(contexto_paquete->limite), sizeof(uint32_t));

    enviar_paquete(paquete, socket_cliente);
    eliminar_paquete(paquete);
    free(contexto_paquete);
}

int conectar_con_filesystem()
{
    // Cliente de FileSystem
    int conexion_filesystem = crear_conexion(memoria_config->ip_filesystem, memoria_config->puerto_filesystem);
    log_debug(memoria_logger, "Conexion con FILESYSTEM creada con exito");
    enviar_handshake(conexion_filesystem, HANDSHAKE_MEMORIA);
    op_code handshake_respuesta = recibir_entero(conexion_filesystem);
    if (handshake_respuesta != HANDSHAKE_ACCEPTED)
    {
        log_error(memoria_logger, "No se pudo conectar con el servidor de FILESYSTEM");
        exit(1);
    }
    return conexion_filesystem;
}