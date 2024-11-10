#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "alarm.h"

// Variable global para el tiempo de la alarma
extern int alarm_time;

// Variable global para debug
extern bool debug;

// Estructura para pasar múltiples argumentos al hilo
typedef struct {
    int *buffer;
    int capacidad;
    int frente;
    int fondo;
    int tam_actual;
    pthread_mutex_t mutex;
} ColaCircular;

// Funciones de inicialización y finalización
ColaCircular* crearColaCircular(int tam_inicial);
void destruirColaCircular(ColaCircular *cola);

// Funciones de productor y consumidor
void agregarElemento(ColaCircular *cola, int item);
int extraerElemento(ColaCircular *cola);

// Funciones auxiliares
void duplicarCola(ColaCircular *cola);
void reducirCola(ColaCircular *cola);
void registrarCambioTamano(const char *accion, int tam_nuevo);

void* productor(void* arg);
void* consumidor(void* arg);

// Crear cola
ColaCircular* crearColaCircular(int tam_inicial) {
    ColaCircular *cola = (ColaCircular *)malloc(sizeof(ColaCircular));
    cola->capacidad = tam_inicial;
    cola->tam_actual = 0;
    cola->frente = 0;
    cola->fondo = 0;
    cola->buffer = (int *)malloc(sizeof(int) * tam_inicial);
    pthread_mutex_init(&cola->mutex, NULL);

    // Crear archivo de log vacío
    FILE *log = fopen("cola_cambios.log", "w");
    if (log) {
        fclose(log);
    }

    if (debug) {
        printf("Se creó la cola con capacidad inicial de %d\n", tam_inicial);
    }

    return cola;
}

// Destruir cola
void destruirColaCircular(ColaCircular *cola) {
    free(cola->buffer);
    pthread_mutex_destroy(&cola->mutex);
    free(cola);

    if (debug) {
        printf("Se destruyó la cola\n");
    }
}

// Agregar elemento
void agregarElemento(ColaCircular *cola, int item) {
    pthread_mutex_lock(&cola->mutex);
    while (cola->tam_actual == cola->capacidad) {
        duplicarCola(cola);
        registrarCambioTamano("duplicar", cola->capacidad);
    }
    cola->buffer[cola->fondo] = item;
    cola->fondo = (cola->fondo + 1) % cola->capacidad;
    cola->tam_actual++;
    pthread_mutex_unlock(&cola->mutex);

    if (debug) {
        printf("Se agregó un elemento a la cola\n");
    }
}

int extraerElemento(ColaCircular *cola) {
    pthread_mutex_lock(&cola->mutex);
    pthread_t alarma_hilo;
    bool alarma_creada = false;
    if (cola->tam_actual <= 0) {
        // Iniciar la alarma si no se puede extraer un elemento
        ThreadArgs args;
        args.sleep_time = alarm_time;
        if (pthread_create(&alarma_hilo, NULL, Alarm, &args) == 0) {
            alarma_creada = true;
        }
    }
    while(cola->tam_actual <= cola->capacidad / 4 && cola->capacidad > 1) {
        reducirCola(cola);
        registrarCambioTamano("reducir", cola->capacidad);
    }
    if(cola->tam_actual <= 0) {
        pthread_mutex_unlock(&cola->mutex);
        return -1;
    }
    int item = cola->buffer[cola->frente];
    cola->frente = (cola->frente + 1) % cola->capacidad;
    cola->tam_actual--;
    pthread_mutex_unlock(&cola->mutex);
    // Desactivar alarma cuando se saco un elemento
    if (alarma_creada) {
        Defuse(alarma_hilo);
    }

    if (debug) {
        printf("Se extrajo un elemento de la cola. Tam actual: %d, Capacidad: %d\n", cola->tam_actual, cola->capacidad);
    }

    return item;
}
// // Extraer elemento
// int extraerElemento(ColaCircular *cola) {
//     pthread_mutex_lock(&cola->mutex);
//     pthread_t alarma_hilo;
//     if (cola->tam_actual == 0) {
//         // Iniciar la alarma si no se puede extraer un elemento
//         ThreadArgs args;
//         args.sleep_time = alarm_time;
//         pthread_create(&alarma_hilo, NULL, Alarm, &args);
//     }
//     while(cola->tam_actual <= cola->capacidad / 4 && cola->capacidad > 1) {
//         reducirCola(cola);
//         registrarCambioTamano("reducir", cola->capacidad);
//     }
//     int item = cola->buffer[cola->frente];
//     cola->frente = (cola->frente + 1) % cola->capacidad;
//     cola->tam_actual--;
//     pthread_mutex_unlock(&cola->mutex);
//     //Desactivar alarma cuando se saco un elemento
//     Defuse(alarma_hilo);

//     if (debug) {
//         printf("Se extrajo un elemento de la cola\n");
//     }

//     return item;
// }

// Duplicar tamaño de la cola
void duplicarCola(ColaCircular *cola) {
    int nueva_capacidad = cola->capacidad * 2;
    int *nuevo_buffer = (int *)malloc(sizeof(int) * nueva_capacidad);
    for (int i = 0; i < cola->tam_actual; i++) {
        nuevo_buffer[i] = cola->buffer[(cola->frente + i) % cola->capacidad];
    }
    free(cola->buffer);
    cola->buffer = nuevo_buffer;
    cola->capacidad = nueva_capacidad;
    cola->frente = 0;
    cola->fondo = cola->tam_actual;

    if (debug) {
        printf("Se duplicó el tamaño de la cola a %d\n", nueva_capacidad);
    }
}

// Reducir tamaño de la cola
void reducirCola(ColaCircular *cola) {
    int nueva_capacidad = cola->capacidad / 2;
    int *nuevo_buffer = (int *)malloc(sizeof(int) * nueva_capacidad);
    for (int i = 0; i < cola->tam_actual; i++) {
        nuevo_buffer[i] = cola->buffer[(cola->frente + i) % cola->capacidad];
    }
    free(cola->buffer);
    cola->buffer = nuevo_buffer;
    cola->capacidad = nueva_capacidad;
    cola->frente = 0;
    cola->fondo = cola->tam_actual;

    if (debug) {
        printf("Se redujo el tamaño de la cola a %d\n", nueva_capacidad);
    }
}

// Registrar cambios de tamaño
void registrarCambioTamano(const char *accion, int tam_nuevo) {
    FILE *log = fopen("cola_cambios.log", "a");
    if (log) {
        time_t ahora = time(NULL);
        fprintf(log, "[%s] Tamaño de la cola %s a %d elementos.\n", ctime(&ahora), accion, tam_nuevo);
        fclose(log);
    }

    if (debug) {
        printf("Se registró un cambio de tamaño: %s a %d\n", accion, tam_nuevo);
    }
}

void* productor(void* arg) {
    ColaCircular *cola = (ColaCircular*) arg;
    int item;
    for(int i = 0; i < 10; i++) {
        agregarElemento(cola, item);
    }

    if (debug) {
        printf("Productor terminó de agregar elementos\n");
    }

    return NULL;
}

void* consumidor(void* arg) {
    ColaCircular *cola = (ColaCircular*) arg;
    int item;
    while(1) {
        if (alarm_triggered) {
            break;
        }
        item = extraerElemento(cola);
    }

    if (debug) {
        printf("Consumidor terminó de extraer elementos\n");
    }

    return NULL;
}

#endif // MONITOR_H
