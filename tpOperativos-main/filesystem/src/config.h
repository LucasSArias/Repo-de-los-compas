#ifndef FILESYSTEM_CONFIG_H_
#define FILESYSTEM_CONFIG_H_

#include <stdlib.h>
#include <sys/stat.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/constants.h>
#include <utils/utils.h>

typedef struct FilesystemConfig
{
    char *puerto_escucha;
    char *mount_dir;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t retardo_acceso_bloque;
    t_log_level log_level;
} t_filesystem_config;

extern t_filesystem_config *filesystem_config;

void load_config_from_file(char *path);
void destroy_config();
void create_filesystem_config(t_config *config_file);

#endif /* FILESYSTEM_CONFIG_H_ */