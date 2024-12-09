#include <utils/utils.h>

t_log *logger;

t_log *iniciar_logger(char *file, char *process_name, t_log_level log_level) // Inicia logger y verifica error.
{
	t_log *nuevo_logger = log_create(file, process_name, true, log_level);

	if (nuevo_logger == NULL)
	{
		perror("Error: No se pudo crear el logger");
		exit(EXIT_FAILURE);
	}

	return nuevo_logger;
}

int crear_socket(struct addrinfo *server_info) // Crea socket y verifica error.
{
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);
	if (socket_cliente == -1)
	{
		perror("Error al crear el socket");
		exit(EXIT_FAILURE);
	}
	return socket_cliente;
}

int crear_conexion(char *ip, char *puerto) // Crea conexion con un socket
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = crear_socket(server_info);

	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_TID_PID(int tid, int pid, int socket_cliente, op_code codigo)
{

	t_paquete *paquete = iniciar_paquete(codigo);

	paquete->buffer->size = sizeof(int) * 2; // Tamaño del buffer para el entero
	paquete->buffer->stream = malloc(paquete->buffer->size);
	int offset = 0;
	memcpy(paquete->buffer->stream, &pid, sizeof(int));
	offset += sizeof(int);
	memcpy(paquete->buffer->stream + offset, &tid, sizeof(int));

	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

t_paquete *iniciar_paquete(int cod_op)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->codigo_operacion = cod_op;
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
	return paquete;
}

t_buffer *inicializar_buffer()
{
	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	buffer->stream = NULL;
	return buffer;
}

void *serializar_paquete(t_paquete *paquete, int bytes) // Modifica el paquete para poder enviarlo via Red. Analiza posibles errores.
{
	void *magic = malloc(bytes);

	if (magic == NULL)
	{
		perror("Error al asignar memoria para la serialización del paquete\n");
		exit(EXIT_FAILURE);
	}

	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete *paquete) // Libera memoria del paquete.
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente) // Envia el paquete via socket
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

int iniciar_servidor(char *puerto)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);
	// Permite que el socket se reutilice en caso de que haya cerrado un servidor previamente
	// Pero podria tener 2 servidores escuchando en el mismo puerto OJO 
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	return socket_servidor;
}

// FUNCIONES PARA CONTEXTO

void eliminar_buffer(t_buffer *buffer)
{
	if (buffer != NULL)
		free(buffer->stream);

	free(buffer);
}

op_code recv_cliente_identificacion(int socket)
{
	op_code cliente_id;
	recv(socket, &cliente_id, sizeof(cliente_id), 0);
	return cliente_id;
}

void send_cliente_identificacion(int socket, op_code cliente)
{
	enviar_entero(socket, cliente);
}

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	return accept(socket_servidor, NULL, NULL);
}

void enviar_entero(int socket_cliente, int numero)
{
	send(socket_cliente, &numero, sizeof(numero), MSG_WAITALL);
}

int recibir_entero(int fd_conexion)
{
	int entero;
	recv(fd_conexion, &entero, sizeof(int), MSG_WAITALL);
	return entero;
}

char *validate_and_get_path(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Debe especificar el path del archivo de configuracion.\n");
		exit(EXIT_FAILURE);
	}
	return argv[1];
}

void enviar_respuesta_con_mensaje(int socket_cliente, op_code codigo_operacion, char *content)
{
	t_paquete *paquete = iniciar_paquete(codigo_operacion);
	paquete->buffer->size = strlen(content) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, content, paquete->buffer->size);

	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

op_code recibir_operacion_from_paquete(int socket_cliente)
{
	op_code codigo_operacion;
	recv(socket_cliente, &codigo_operacion, sizeof(codigo_operacion), 0);
	return codigo_operacion;
}

void enviar_handshake(int socket_cliente, op_code codigo_operacion)
{
	enviar_entero(socket_cliente, codigo_operacion);
}

char* syscall_to_string(syscall_t syscall){
    switch(syscall){
        case S_PROCESS_CREATE:
            return "PROCESS_CREATE";
        case S_PROCESS_EXIT:
            return "PROCESS_EXIT";
        case S_THREAD_CREATE:
            return "THREAD_CREATE";
        case S_THREAD_JOIN:
            return "THREAD_JOIN";
        case S_THREAD_CANCEL:
            return "THREAD_CANCEL";
        case S_THREAD_EXIT:
            return "THREAD_EXIT";
        case S_MUTEX_CREATE:
            return "MUTEX_CREATE";
        case S_MUTEX_LOCK:
            return "MUTEX_LOCK";
        case S_MUTEX_UNLOCK:
            return "MUTEX_UNLOCK";
        case S_DUMP_MEMORY:
            return "DUMP_MEMORY";
        case S_IO:
            return "IO";
        default:
            return "SYSCALL NO RECONOCIDA";
    }
}
