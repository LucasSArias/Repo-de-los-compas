#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <semaphore.h>
#include <pthread.h>


int crear_conexion(char* ip, char* puerto);



typedef enum op_code
{
	// Interrupcion y syscalls que desatan en replaning
	END_OF_QUANTUM,
	BLOCK_THREAD_JOIN,
	BLOCK_IO,
	BLOCK_BY_MUTEX,
	BLOCK_BY_DUMP,
	END_BY_THREAD_CANCEL,
	END_BY_THREAD_EXIT,
	END_BY_PROCESS_EXIT,
	END_BY_SEGMENTATION_FAULT,
	NOT_BLOCK_THREAD_JOIN,
	NOT_BLOCK_BY_MUTEX,
	NOT_END_BY_THREAD_CANCEL,
	// Syscalls
    ERROR_CK,
	PROCESS_CREATE,
    PROCESS_EXIT,
    THREAD_CREATE,
    THREAD_JOIN,
    THREAD_CANCEL,
    THREAD_EXIT,
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
    DUMP_MEMORY,
    IO,
	EJECUTAR_HILO,
    SYSCALL,
	/*HANDSHAKES*/
	HANDSHAKE_KERNEL,
	HANDSHAKE_CPU,
	HANDSHAKE_MEMORIA,
	HANDSHAKE_FILESYSTEM,
	HANDSHAKE_ACCEPTED,
	HANDSHAKE_DENIED,
	/*CPU->MEMORIA*/
	GET_CONTEXTO_EJECUCION,
	UPD_CONTEXTO_EJECUCION,
	GET_INSTRUCCION,
	READ_MEM,
	WRITE_MEM,
	/*KERNEL->MEMORIA*/
	CREATE_PROCESS,
	END_PROCESS,
	CREATE_THREAD,
	END_THREAD,
	MEMORY_DUMP,
	/*MEMORIA->FILESYSTEM*/
	WRITE_FILE,
	/*GENERALES*/
	OK,
	ERROR
} op_code;

typedef enum{
    S_PROCESS_CREATE,
    S_PROCESS_EXIT,
    S_THREAD_CREATE,
    S_THREAD_JOIN,
    S_THREAD_CANCEL,
    S_THREAD_EXIT,
    S_MUTEX_CREATE,
    S_MUTEX_LOCK,
    S_MUTEX_UNLOCK,
    S_DUMP_MEMORY,
    S_IO
} syscall_t;

char* syscall_to_string(syscall_t syscall);

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_log* iniciar_logger(char* file, char *process_name, t_log_level log_level); // Inicia un logger pasanodole: Archivo, Nombre, Nivel de LOG.
int crear_socket(struct addrinfo *server_info); // Crea sockey y verifica error.
int crear_conexion(char* ip, char* puerto); // Crea la conexion usando el IP y Puerto. Es de CLIENTE -> SERVER
void enviar_TID_PID(int tid, int pid, int socket_cliente, op_code codigo); // Utilizado para enviar TIDs y PIDs recibidos por parametro
t_buffer* inicializar_buffer();
void* serializar_paquete(t_paquete* paquete, int bytes); // Modifica el paquete para poder enviarlo via Red. 
void eliminar_paquete(t_paquete* paquete); // Libera memoria del paquete.
void enviar_paquete(t_paquete* paquete, int socket_cliente); // Elimina y libera memoria de paquete
int iniciar_servidor(char* puerto); // 
int esperar_cliente(int socket_servidor);
t_paquete* iniciar_paquete(int cod_op);
op_code recv_cliente_identificacion(int socket);
void send_cliente_identificacion(int socket, op_code cliente);
void enviar_entero(int socket_cliente, int numero);
int recibir_entero(int fd_conexion);
void eliminar_buffer(t_buffer *buffer);
char *validate_and_get_path(int argc, char *argv[]);
void enviar_respuesta_con_mensaje(int socket_cliente, op_code codigo_operacion, char *content);
op_code recibir_operacion_from_paquete(int socket_cliente);
void enviar_handshake(int socket_cliente, op_code codigo_operacion);

#endif