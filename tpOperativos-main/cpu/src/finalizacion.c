#include "finalizacion.h"

void destroy_config()
{
    free(cpu_config->puerto_escucha_dispatch);
    free(cpu_config->puerto_escucha_interrupt);
    free(cpu_config->ip_memoria);
    free(cpu_config->puerto_memoria);
    free(cpu_config);
}

void finalizar_cpu()
{
    log_info(cpu_logger, "Finalizando CPU");
    log_destroy(cpu_logger);
    destroy_config();
    cerrar_conexiones_cpu();
}
