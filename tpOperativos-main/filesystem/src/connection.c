#include <connection.h>

void iniciar_servidor_filesystem()
{
    int server_filesystem = iniciar_servidor(filesystem_config->puerto_escucha);
    if (server_filesystem == -1)
    {
        log_error(filesystem_logger, "Error al iniciar el servidor");
        return;
    }

    log_debug(filesystem_logger, "Servidor escuchando en el puerto: %s", filesystem_config->puerto_escucha);

    while (1)
    {
        int cliente = esperar_cliente(server_filesystem);
        if (cliente != -1)
        {
            log_debug(filesystem_logger, "Nueva conexión recibida, esperando a que se identifique el cliente");

            pthread_t hilo_cliente;
            int *cliente_ptr = malloc(sizeof(*cliente_ptr));
            if (cliente_ptr == NULL)
            {
                log_error(filesystem_logger, "Error al asignar memoria para el cliente");
                close(cliente);
                continue;
            }
            *cliente_ptr = cliente;

            if (pthread_create(&hilo_cliente, NULL, manejar_cliente, (void *)cliente_ptr) != 0)
            {
                log_error(filesystem_logger, "Error al crear el hilo para el cliente");
                free(cliente_ptr);
                close(cliente);
            }
            else
            {
                pthread_detach(hilo_cliente); // Desvincular el hilo para que se limpie automáticamente al terminar
            }
        }
        else
        {
            log_error(filesystem_logger, "Error al esperar cliente");
        }
    }

    // Cerrar el servidor cuando salgas del bucle
    close(server_filesystem);
    log_info(filesystem_logger, "Servidor finalizado");
}

void *manejar_cliente(void *socketCliente)
{
    int cliente = *((int *)socketCliente);
    free(socketCliente);
    op_code cliente_id = recv_cliente_identificacion(cliente);
    switch (cliente_id)
    {
    case HANDSHAKE_MEMORIA:
        log_debug(filesystem_logger, "Se conecto %s", MEMORIA);
        enviar_handshake(cliente, HANDSHAKE_ACCEPTED);
        manejar_cliente_memoria(cliente);
        break;
    default:
        log_warning(filesystem_logger, "No se pudo identificar al cliente; op_code: %d", cliente_id);
        break;
    }

    return NULL;
}

void manejar_cliente_memoria(int socket)
{
    op_code operacion = recibir_operacion_from_paquete(socket);
    switch (operacion)
    {
    case WRITE_FILE:
        t_write_file_request *write_file_request = recibir_write_file_request(socket);
        creacion_de_archivo(write_file_request, socket);
        enviar_entero(socket, OK);
        shutdown(socket, SHUT_RDWR);
        close(socket);
        log_obligatorio_fin_peticion(write_file_request->filename);
        liberar_write_file_request(write_file_request);
        break;
    default:
        log_warning(filesystem_logger, "Operación desconocida. op_code: %d", operacion);
        break;
    }
}

void creacion_de_archivo(t_write_file_request *file_request, int socket)
{
    int cantidad_bloques = ((file_request->tamanio + (filesystem_config->block_size - 1)) / filesystem_config->block_size) + 1;
    if (!validar_cantidad_punteros_x_bloque(cantidad_bloques))
    {
        enviar_entero(socket, ERROR);
        log_error(filesystem_logger, "No es posible direccionar la totalidad del archivo en un bloque.");
        return;
    }
    if (!evaluar_espacio(file_request->tamanio, cantidad_bloques))
    {
        enviar_entero(socket, ERROR);
        log_error(filesystem_logger, "No hay espacio disponible para la creacion del archivo.");
        return;
    }
    uint32_t *indices = reservar_bloques(file_request->tamanio, cantidad_bloques, file_request->filename); // bloques de indice y de datos;
    t_metadata *metadata = malloc(sizeof(t_metadata));
    metadata->tamanio = file_request->tamanio;
    metadata->indice_bloque = indices[0];
    escribir_metadata(file_request->filename, metadata);
    grabar_bloque_indice(indices, file_request->filename, cantidad_bloques);
    grabar_bloques(indices, file_request, cantidad_bloques);
    free(indices);
}