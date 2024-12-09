#ifndef FILESYSTEM_METADATA_H_
#define FILESYSTEM_METADATA_H_

#include "utils/file.h"
#include "config.h"
#include "logger.h"

typedef struct Metadata
{
    uint32_t indice_bloque; //no deberia ser tipo puntero? No, es numerico. Seria el indice del array de bloques
    uint32_t tamanio;
} t_metadata;

//t_metadata *leer_metadata(char *file_name);
void escribir_metadata(char *file_name, t_metadata *metadata);

#endif /* FILESYSTEM_METADATA_H_ */