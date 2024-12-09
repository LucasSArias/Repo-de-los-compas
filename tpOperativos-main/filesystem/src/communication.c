#include <communication.h>

t_write_file_request *recibir_write_file_request(int socket_cliente)
{
    t_buffer *buffer = inicializar_buffer();

    // Recibir el tamaño total del buffer y asignar el espacio en `stream`
    recv(socket_cliente, &(buffer->size), sizeof(int), MSG_WAITALL);
    buffer->stream = malloc(buffer->size);

    // Recibir todo el contenido en el `stream`
    recv(socket_cliente, buffer->stream, buffer->size, MSG_WAITALL);

    // Crear la estructura `t_write_file_request`
    t_write_file_request *write_file_request = malloc(sizeof(t_write_file_request));
    
    int offset = 0;
    size_t filename_len;
    size_t content_len;

    memcpy(&filename_len, buffer->stream + offset, sizeof(size_t));
    offset += sizeof(size_t);
    write_file_request->filename = malloc(filename_len + 1);
    memcpy(write_file_request->filename, buffer->stream + offset, filename_len);
    offset += filename_len;
    write_file_request->filename[filename_len] = '\0';

    memcpy(&content_len, buffer->stream + offset, sizeof(size_t));
    offset += sizeof(size_t);
    write_file_request->content = malloc(content_len);
    memcpy(write_file_request->content, buffer->stream + offset, content_len);
    offset += content_len;
    //write_file_request->content[content_len] = '\0';

    write_file_request->tamanio = content_len;

    // Liberar el buffer de `stream`
    eliminar_buffer(buffer);

    return write_file_request;
}

void liberar_write_file_request(t_write_file_request *write_file_request)
{
    free(write_file_request->filename);
    free(write_file_request->content);
    free(write_file_request);
}