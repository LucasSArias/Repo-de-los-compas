#include <memoria.h>

int main(int argc, char *argv[])
{
    //config, el path por default se lee desde .vscode/launch.json 
    load_config_from_file(validate_and_get_path(argc, argv));
    //logger
    iniciar_logger_memoria();
    //estructura de memoria, hilos y particiones
    inicializar_memoria();

    pthread_mutex_init(&partition_mutex, NULL);
    pthread_mutex_init(&hilos_mutex, NULL);
    
    //servidor de memoria
    iniciar_servidor_memoria();

    terminar_memoria();

    return EXIT_SUCCESS;
}

void iniciar_servidor_memoria()
{
    int server_memoria = iniciar_servidor(memoria_config->puerto_escucha);
    if (server_memoria == -1)
    {
        log_error(memoria_logger, "Error al iniciar el servidor");
        return;
    }

    log_debug(memoria_logger, "Servidor escuchando en el puerto: %s", memoria_config->puerto_escucha);

    while (1)
    {
        int cliente = esperar_cliente(server_memoria);
        if (cliente != -1)
        {
            pthread_t hilo_cliente;
            int *cliente_ptr = malloc(sizeof(*cliente_ptr));
            if (cliente_ptr == NULL)
            {
                log_error(memoria_logger, "Error al asignar memoria para el cliente");
                close(cliente);
                continue;
            }
            *cliente_ptr = cliente;

            if (pthread_create(&hilo_cliente, NULL, manejar_cliente, (void *)cliente_ptr) != 0)
            {
                log_error(memoria_logger, "Error al crear el hilo para el cliente");
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
            log_error(memoria_logger, "Error al esperar cliente");
        }
    }

    // Cerrar el servidor cuando salgas del bucle
    close(server_memoria);
    log_info(memoria_logger, "Servidor finalizado");
}

void *manejar_cliente(void *socketCliente)
{
    int cliente = *((int *)socketCliente);
    free(socketCliente);
    op_code cliente_id = recv_cliente_identificacion(cliente);
    switch (cliente_id)
    {
    case HANDSHAKE_KERNEL:
        log_obligatorio_conex_kernel(cliente);
        enviar_handshake(cliente, HANDSHAKE_ACCEPTED);
        manejar_cliente_kernel(cliente);
        break;
    case HANDSHAKE_CPU:
        log_debug(memoria_logger, "Se conecto CPU");
        validate_cpu_connection(cliente);
        manejar_cliente_cpu(cliente);
        break;
    default:
        log_warning(memoria_logger, "No se pudo identificar al cliente; op_code: %d", cliente_id);
        break;
    }
    shutdown(cliente, SHUT_RDWR);
    close(cliente);
    return NULL;
}

void manejar_cliente_kernel(int socket)
{
    op_code operacion = recibir_operacion_from_paquete(socket);
    switch (operacion)
    {
    case CREATE_PROCESS:
        t_proceso_paquete *proceso_paquete = recibir_proceso(socket);
        log_obligatorio_proceso(CREACION, proceso_paquete->pid, proceso_paquete->tamanio);
        crear_proceso(socket, proceso_paquete);
        //en crear_proceso ya se esta enviando la respuesta
        break;
    case END_PROCESS:
        int pid = recibir_id(socket);
        log_obligatorio_proceso(DESTRUCCION, pid, tamanio_particion_por_pid(pid));
        finalizar_proceso(pid);
        enviar_estado_solicitud(socket, OK);
        break;
    case CREATE_THREAD:
        t_hilo_paquete *hilo_paquete = recibir_hilo(socket);
        log_obligatorio_hilo(CREACION, hilo_paquete->pid, hilo_paquete->tid);
        crear_contexto_hilo(hilo_paquete);
        enviar_estado_solicitud(socket, OK);
        break;
    case END_THREAD:
        t_thread_to_end_request *thread_to_end_request = recibir_thread_to_end(socket);
        log_obligatorio_hilo(DESTRUCCION, thread_to_end_request->pid, thread_to_end_request->tid);
        finalizar_hilo(thread_to_end_request->pid, thread_to_end_request->tid);
        enviar_estado_solicitud(socket, OK);
        free(thread_to_end_request);
        break;
    case MEMORY_DUMP:
        t_memory_dump_request *memory_dump_request = recibir_memory_dump_request(socket);
        log_obligatorio_memory_dump(memory_dump_request->pid, memory_dump_request->tid);
        memory_dump(memory_dump_request->pid, memory_dump_request->tid, socket);
        //en memory_dump ya se esta enviando la respuesta
        free(memory_dump_request);
        break;
    default:
        log_warning(memoria_logger, "Operación no manejable en kernel. op_code: %d", operacion);
        break;
    }
}

void manejar_cliente_cpu(int socket)
{
    int crashes = 0;
    while (1)
    {
        if(crashes >= 3){
            log_warning(memoria_logger, "Es probable que el servidor de CPU se haya cerrado. Se aborta el manejo de cliente");
            close(socket);
            exit(EXIT_FAILURE);
        }
        op_code operacion = recibir_operacion_from_paquete(socket);
        switch (operacion)
        {
        case GET_CONTEXTO_EJECUCION:
            t_get_contexto_ejecucion_request *get_contexto_ejecucion_request = recibir_get_contexto_ejecucion_request(socket);
            log_obligatorio_contexto(SOLICITADO, get_contexto_ejecucion_request->pid, get_contexto_ejecucion_request->tid);
            t_new_contexto_paquete *contexto_ejecucion = obtener_contexto_hilo(get_contexto_ejecucion_request->pid, get_contexto_ejecucion_request->tid);
            poblar_contexto_hilo_con_particion(contexto_ejecucion, get_contexto_ejecucion_request->pid);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_contexto_hilo(socket, OK, contexto_ejecucion);
            free(get_contexto_ejecucion_request);
            break;
        case UPD_CONTEXTO_EJECUCION:
            t_upd_contexto_paquete *upd_contexto_paquete = recibir_upd_contexto(socket);
            log_obligatorio_contexto(ACTUALIZADO, upd_contexto_paquete->pid, upd_contexto_paquete->tid);
            actualizar_contexto_hilo(upd_contexto_paquete);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_estado_solicitud(socket, OK);
            free(upd_contexto_paquete);
            break;
        case GET_INSTRUCCION:
            t_get_instruccion_request *get_instruccion_request = recibir_get_instruccion_request(socket);
            char *instruccion = obtener_instruccion_with_PC(get_instruccion_request->pid, get_instruccion_request->tid, get_instruccion_request->pc);
            log_obligatorio_instruccion(get_instruccion_request->pid, get_instruccion_request->tid, instruccion);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            if(string_contains(instruccion, "ERROR"))
            {
                enviar_estado_solicitud(socket, ERROR);
                break;
            }
            enviar_instruccion_a_cpu(instruccion, socket);
            free(get_instruccion_request);
            break;
        case READ_MEM:
            //CPU envia pid, tid y direccion fisica. Pero no valido la direccion fisica
            t_memory_read_request *read_mem_request = recibir_memory_read_request(socket);
            log_obligatorio_memoria_usuario(LECTURA, read_mem_request->pid, read_mem_request->tid,
                                            read_mem_request->direccion_fisica, BYTES_PER_OPERATION);
            void *buffer = malloc(BYTES_PER_OPERATION);
            leer_memoria(read_mem_request->direccion_fisica, buffer);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_respuesta_lectura_memoria_OK(socket, buffer);
            free(read_mem_request);
            break;
        case WRITE_MEM:
            //CPU envia pid, tid, direccion fisica y los bytes a escribir. Pero no valido la direccion fisica
            t_memory_write_request *write_mem_request = recibir_memory_write_request(socket);
            log_obligatorio_memoria_usuario(ESCRITURA, write_mem_request->pid, write_mem_request->tid,
                                            write_mem_request->direccion_fisica, BYTES_PER_OPERATION);
            escribir_memoria(write_mem_request->direccion_fisica, write_mem_request->buffer);
            sleep_milliseconds(memoria_config->retardo_respuesta);
            enviar_estado_solicitud(socket, OK);
            free(write_mem_request->buffer);
            free(write_mem_request);
            break;
        default:
            log_warning(memoria_logger, "Operación no manejable en CPU. op_code: %d", operacion);
            crashes++;
            break;
        }
    }
}

void terminar_memoria()
{
    log_destroy(memoria_logger);
    destroy_config();
    destroy_memoria();
}

void validate_cpu_connection(int socket_cliente)
{
    if (is_cpu_connected)
    {
        enviar_handshake(socket_cliente, HANDSHAKE_DENIED);
        close(socket_cliente);
    }
    else
    {
        enviar_handshake(socket_cliente, HANDSHAKE_ACCEPTED);
    }
}