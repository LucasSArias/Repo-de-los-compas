#ifndef MEMORIA_COMUNICACION_H_
#define MEMORIA_COMUNICACION_H_

#include <commons/log.h>
#include <utils/utils.h>
#include <utils/file.h>
#include <commons/string.h>
#include <unistd.h>
#include <config.h>

typedef struct MemoryDumpRequest
{
    uint32_t pid;
    uint32_t tid;
} t_memory_dump_request;

typedef struct MemoryReadRequest
{
    uint32_t pid;
    uint32_t tid;
    uint32_t direccion_fisica;
} t_memory_read_request;

typedef struct MemoryWriteRequest
{
    uint32_t pid;
    uint32_t tid;
    uint32_t direccion_fisica;
    void *buffer;
} t_memory_write_request;

typedef struct GetContextoEjecucionRequest {
    uint32_t pid;
    uint32_t tid;
} t_get_contexto_ejecucion_request;

typedef struct GetInstruccionRequest {
    uint32_t pid;
    uint32_t tid;
    uint32_t pc;
} t_get_instruccion_request;

typedef struct ThreadToEndRequest {
    uint32_t pid;
    uint32_t tid;
} t_thread_to_end_request;

typedef struct HiloPaquete {
	uint32_t pid;
	uint32_t tid;
	t_list *instrucciones;
} t_hilo_paquete;

typedef struct ProcesoPaquete {
	uint32_t pid;
	uint32_t tamanio;
	t_list *instrucciones;
} t_proceso_paquete;

typedef struct NewContextoPaquete {
	uint32_t ax, bx, cx, dx, ex, fx, gx, hx;
	uint32_t pc;
	uint32_t base;
	uint32_t limite;
} t_new_contexto_paquete;

typedef struct UpdContextoPaquete {
	uint32_t pid;
	uint32_t tid;
	uint32_t ax, bx, cx, dx, ex, fx, gx, hx;
	uint32_t pc;
} t_upd_contexto_paquete;

#include <memoria_administracion.h>

t_hilo_paquete *recibir_hilo(int socket_cliente);
t_proceso_paquete *recibir_proceso(int socket_cliente);
int recibir_id(int socket_cliente);
t_upd_contexto_paquete *recibir_upd_contexto(int socket_cliente);
t_memory_dump_request *recibir_memory_dump_request(int socket_cliente);
char *recibir_respuesta_de_filesystem(int socket_filesystem);
t_memory_read_request *recibir_memory_read_request(int socket_cliente);
t_memory_write_request *recibir_memory_write_request(int socket_cliente);
t_get_contexto_ejecucion_request *recibir_get_contexto_ejecucion_request(int socket_cliente);
t_get_instruccion_request *recibir_get_instruccion_request(int socket_cliente);
t_thread_to_end_request *recibir_thread_to_end(int socket_cliente);

void enviar_contexto_hilo(int socket_cliente, op_code codigo_operacion, t_new_contexto_paquete *contexto_paquete);
void enviar_estado_solicitud(int socket_cliente, op_code codigo_operacion);
void enviar_solicitud_memory_dump(char *filename, void *content, int conexion_filesystem, size_t size_content);
void enviar_respuesta_lectura_memoria_OK(int socket_cliente, void *buffer);
void enviar_instruccion_a_cpu(char *instruccion, int socket_cliente);

void sleep_milliseconds(unsigned int milliseconds);

int conectar_con_filesystem();

#endif /* MEMORIA_COMUNICACION_H_ */