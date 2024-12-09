#ifndef CPU_INICIALIZACION_H_
#define CPU_INICIALIZACION_H_

#include "bibliotecas.h"

#include "conexiones.h"

extern t_cpu_config *cpu_config;

extern t_log* cpu_logger;

void inicializar_cpu(char *path);
void create_cpu_config(t_config *config_file);
void load_config_from_file(char *path);
void iniciar_logger_cpu();

#endif /* CPU_INICIALIZACION_H_ */