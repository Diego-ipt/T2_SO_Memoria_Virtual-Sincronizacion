#ifndef ALARM_H
#define ALARM_H

#include <stdio.h>
#include <unistd.h> 
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

// Estructura para pasar múltiples argumentos al hilo
typedef struct {
    int sleep_time;
} ThreadArgs;

// Variable global para la bandera
extern bool alarm_triggered;

// Variable global para debug
extern bool debug;

// Mutex para proteger el acceso a la bandera
extern pthread_mutex_t alarm_mutex;

// Función para cancelar un hilo
void Defuse(pthread_t thread);

// Función que ejecutará el hilo
void* Alarm(void* arg);

// Función para reiniciar un hilo
void RestartAlarm(pthread_t* thread, ThreadArgs* args);

// Función para imprimir el estado de la bandera
void* PrintFlagStatus(void* arg);

// Función para cancelar un hilo
void Defuse(pthread_t thread) {
    int result = pthread_cancel(thread);
    if (result == 0) {
        if (debug) {
            printf("Hilo %lu cancelado.\n", thread);
        }
    } else {
        printf("Error al cancelar el hilo %lu.\n", thread);
    }
}

// Función que ejecutará el hilo
void* Alarm(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    pthread_t id = pthread_self();
    if (debug) {
        printf("Hilo %lu iniciado, durmiendo por %d segundos.\n", id, args->sleep_time);
    }
    sleep(args->sleep_time);
    // Marcar la bandera en true
    pthread_mutex_lock(&alarm_mutex);
    alarm_triggered = true;
    pthread_mutex_unlock(&alarm_mutex);
    if (debug) {
        printf("Alarma activada por el hilo %lu.\n", id);
    }

    return NULL;
}

// Función para reiniciar un hilo
void RestartAlarm(pthread_t* thread, ThreadArgs* args) {
    // Cancelar el hilo actual
    Defuse(*thread);
    
    // Crear un nuevo hilo con los mismos argumentos
    pthread_create(thread, NULL, Alarm, args);
    if (debug) {
        printf("Hilo %lu reiniciado.\n", *thread);
    }
}

void* PrintFlagStatus(void* arg) {
    while (true) {
        pthread_mutex_lock(&alarm_mutex);
        bool status = alarm_triggered;
        pthread_mutex_unlock(&alarm_mutex);

        printf("Estado de la bandera: %s\n", status ? "true" : "false");
        usleep(500000); // 0.5 segundos
    }
    return NULL;
}

bool* Return_flag_status(void* arg) {
    static bool status;
    while (true) {
        pthread_mutex_lock(&alarm_mutex);
        status = alarm_triggered;
        if(status==1) {
            pthread_mutex_unlock(&alarm_mutex);
            return &status;
        }
        pthread_mutex_unlock(&alarm_mutex);
        usleep(500000); // 0.5 segundos
    }
    return NULL;
}

// Función para convertir la bandera a false
void ResetFlag() {
    pthread_mutex_lock(&alarm_mutex);
    alarm_triggered = false;
    pthread_mutex_unlock(&alarm_mutex);
    if (debug) {
        printf("Alarma desactivada.\n");
    }
}

#endif // ALARM_H
