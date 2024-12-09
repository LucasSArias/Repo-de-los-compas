#include "kernel.h"
#include "interrupciones.h"
#include "inicializacion.h"
#include "planificacion.h"
#include "estructuras.h"
#include "syscalls.h"
#include "finalizacion.h"

int main(int argc, char* argv[]) {
    /*
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    */
    inicializar_kernel(validate_and_get_path(argc, argv)/* Path de configs */); // Inicia los loggers y configs

    log_debug(kernel_logger, "Kernel iniciado de forma exitosa");
    PROCESS_CREATE_KERNEL(argv[2]/* path de pseudocodigo */, atoi(argv[3]) /* tam_memoria */, 0); // Inicia proceso base pasandole el archivo de pseudocodigo, el tamaño en memoria y la prioridad maxima del hilo principal (0)

    pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL); // Creo hilo para correr el planificador de largo plazo.
    pthread_detach(hilo_planificador_largo_plazo);

    pthread_create(&hilo_gestionar_syscalls, NULL, gestionar_syscalls, NULL); // Creo hilo para recibir syscalls desde CPU-Dispatch
    pthread_detach(hilo_gestionar_syscalls);

    pthread_create(&hilo_recibir_interrupciones_confirmadas, NULL, recibir_interrupciones_confirmadas, NULL); // Creo hilo para recibir interrupciones confirmadas desde CPU-Interrupt
    pthread_detach(hilo_recibir_interrupciones_confirmadas);

    if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0 || strcmp(kernel_config.ALGORITMO_PLANIFICACION, "CMN") == 0) { // A CHEQUEAR
        colas_prioridad = list_create();
        pthread_create(&hilo_colas_por_prioridad, NULL, crear_colas_por_prioridad, NULL); 
    }
    if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "FIFO") == 0) {
        log_debug(kernel_logger, "Planificador FIFO");
        pthread_t hilo_planificador_fifo;
        pthread_create(&hilo_planificador_fifo, NULL, planificar_fifo_hilos, NULL);        
        pthread_join(hilo_planificador_fifo, NULL);
    } else if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "PRIORIDADES") == 0) {
        log_debug(kernel_logger, "Planificador por prioridades");
        pthread_t hilo_planificiador_prioridades;
        pthread_create(&hilo_planificiador_prioridades, NULL, planificar_prioridades_hilos, NULL);
        pthread_join(hilo_planificiador_prioridades, NULL);
    } else if (strcmp(kernel_config.ALGORITMO_PLANIFICACION, "CMN") == 0) {
        log_debug(kernel_logger, "Planificador de colas multinivel");
        pthread_t hilo_informar_interrupciones_por_QT; 
        pthread_create(&hilo_informar_interrupciones_por_QT, NULL, interrumpir_por_fin_quantum, NULL);
        pthread_detach(hilo_informar_interrupciones_por_QT);
        
        pthread_t hilo_planificador_cmn;
        pthread_create(&hilo_planificador_cmn, NULL, planificar_cmn_hilos, NULL);
        pthread_join(hilo_planificador_cmn, NULL);
    } else {
        log_error(kernel_logger, "Algoritmo de planificacion desconocido: %s", kernel_config.ALGORITMO_PLANIFICACION);
        exit(EXIT_FAILURE);
    }

    //pthread_join(hilo_gestionar_syscalls, NULL);
    //pthread_join(hilo_planificador_largo_plazo, NULL);

    finalizar_kernel();
    exit(EXIT_SUCCESS);
}