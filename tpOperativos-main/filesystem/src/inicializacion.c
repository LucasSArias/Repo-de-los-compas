#include "inicializacion.h"

void iniciar_estructuras_filesystem()
{
    iniciar_logger_filesystem();
    inicializar_bloques();
    inicializar_bitmap();

    pthread_mutex_init(&mutex_bloques, NULL);
    pthread_mutex_init(&mutex_bitmap, NULL);
}