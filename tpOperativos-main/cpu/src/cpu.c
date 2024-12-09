#include "cpu.h"

pthread_mutex_t mutex_var_global_syscall;
pthread_mutex_t mutex_var_global_interrupt;
pthread_mutex_t mutex_var_global_hay_interrupcion;
sem_t llego_la_interrupcion;

int main(int argc, char *argv[])
{
    
    inicializar_cpu(validate_and_get_path(argc, argv)); // Loggers, config, conexiones

    pthread_create(&hilo_interrupt, NULL, atender_cpu_interrupt, NULL); // Recibir interrupciones
    pthread_detach(hilo_interrupt);

    pthread_mutex_init(&mutex_var_global_syscall, NULL);
    pthread_mutex_init(&mutex_var_global_interrupt, NULL);
    pthread_mutex_init(&mutex_var_global_hay_interrupcion, NULL);
    sem_init(&llego_la_interrupcion, 0, 0);

    atender_cpu_dispatch(); // Recibir primer hilo a ejecutar

    finalizar_cpu(); // Cerrar conexiones, loggers, etc.

    return EXIT_SUCCESS;
}