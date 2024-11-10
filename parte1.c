#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include "monitor.h"

bool alarm_triggered = false;
pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
int alarm_time= 0;

int main(int argc, char *argv[]) {
    int num_productores = 0, num_consumidores = 0, tam_inicial = 0, tiempo_espera = 0;
    int opt;

    // Parsear argumentos de entrada
    while ((opt = getopt(argc, argv, "p:c:s:t:")) != -1) {
        switch (opt) {
            case 'p': num_productores = atoi(optarg); break;
            case 'c': num_consumidores = atoi(optarg); break;
            case 's': tam_inicial = atoi(optarg); break;
            case 't': tiempo_espera = atoi(optarg); break;
            default:
                fprintf(stderr, "Uso: %s -p <productores> -c <consumidores> -s <tam_cola> -t <tiempo_espera>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (num_productores <= 0 || num_consumidores <= 0 || tam_inicial <= 0 || tiempo_espera <= 0) {
        fprintf(stderr, "Todos los parámetros deben ser mayores que cero.\n");
        exit(EXIT_FAILURE);
    }
    
    alarm_time = tiempo_espera;

    // Crear la cola compartida
    ColaCircular *cola = crearColaCircular(tam_inicial);

    // Crear hilos de productores
    pthread_t hilos_productores[num_productores];
    for (int i = 0; i < num_productores; i++) {
        if (pthread_create(&hilos_productores[i], NULL, productor, cola) != 0) {
            perror("Error al crear hilo productor");
            exit(EXIT_FAILURE);
        }
    }

    // Crear hilos de consumidores
    pthread_t hilos_consumidores[num_consumidores];
    ConsumidorArgs *consumidor_args[num_consumidores];
    for (int i = 0; i < num_consumidores; i++) {
        consumidor_args[i] = malloc(sizeof(ConsumidorArgs));
        consumidor_args[i]->cola = cola;
        consumidor_args[i]->id = i;
        consumidor_args[i]->tiempo_espera = tiempo_espera;
        if (pthread_create(&hilos_consumidores[i], NULL, consumidor, consumidor_args[i]) != 0) {
            perror("Error al crear hilo consumidor");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que los hilos de productores terminen
    for (int i = 0; i < num_productores; i++) {
        pthread_join(hilos_productores[i], NULL);
    }

    // Bucle para verificar el estado de la variable program_running
    while (alarm_triggered == false) {
        usleep(500000); // 0.5 segundos
    }

    // Esperar a que los hilos de consumidores terminen
    for (int i = 0; i < num_consumidores; i++) {
        free(consumidor_args[i]);
    }

    // Liberar recursos
    destruirColaCircular(cola);

    printf("Fin\n");
    return 0;
}