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

// Estructura para pasar múltiples argumentos al hilo
typedef struct {
    int *buffer;
    int capacidad;
    int frente;
    int fondo;
    int tam_actual;
    int tam_inicial;
    pthread_mutex_t mutex;
    pthread_cond_t puede_producir;
    pthread_cond_t puede_consumir;
} ColaCircular;

typedef struct {
    ColaCircular *cola;
    int id;
    int tiempo_espera;
} ConsumidorArgs;

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
    cola->tam_inicial = tam_inicial;
    cola->tam_actual = 0;
    cola->frente = 0;
    cola->fondo = 0;
    cola->buffer = (int *)malloc(sizeof(int) * tam_inicial);
    pthread_mutex_init(&cola->mutex, NULL);
    pthread_cond_init(&cola->puede_producir, NULL);
    pthread_cond_init(&cola->puede_consumir, NULL);
    return cola;
}

// Destruir cola
void destruirColaCircular(ColaCircular *cola) {
    free(cola->buffer);
    pthread_mutex_destroy(&cola->mutex);
    pthread_cond_destroy(&cola->puede_producir);
    pthread_cond_destroy(&cola->puede_consumir);
    free(cola);
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
    pthread_cond_signal(&cola->puede_consumir);
    pthread_mutex_unlock(&cola->mutex);
}

// Extraer elemento
int extraerElemento(ColaCircular *cola) {
    pthread_mutex_lock(&cola->mutex);
    pthread_t alarma_hilo;
    while (cola->tam_actual == 0) {
        // Iniciar la alarma si no se puede extraer un elemento
        ThreadArgs args;
        args.sleep_time = alarm_time;
        pthread_create(&alarma_hilo, NULL, Alarm, &args);

        pthread_cond_wait(&cola->puede_consumir, &cola->mutex);
    }
    int item = cola->buffer[cola->frente];
    cola->frente = (cola->frente + 1) % cola->capacidad;
    cola->tam_actual--;
    pthread_cond_signal(&cola->puede_producir);
    pthread_mutex_unlock(&cola->mutex);
    Defuse(alarma_hilo);
    return item;
}


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
}

// Registrar cambios de tamaño
void registrarCambioTamano(const char *accion, int tam_nuevo) {
    FILE *log = fopen("log_cola.txt", "a");
    if (log) {
        time_t ahora = time(NULL);
        fprintf(log, "[%s] Tamaño de la cola %s a %d elementos.\n", ctime(&ahora), accion, tam_nuevo);
        fclose(log);
    }
}

void* productor(void* arg) {
    ColaCircular *cola = (ColaCircular*) arg;
    int item;
    agregarElemento(cola, item);
}


void* consumidor(void* arg) {
    ConsumidorArgs *args = (ConsumidorArgs*) arg;
    ColaCircular *cola = args->cola;
    int item;
    while(1){
        item = extraerElemento(cola);
        sleep(1);
    }
}


#endif // MONITOR_H
