#include "alarm.h"

bool alarm_triggered = false;
pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
    pthread_t hilos[5];
    ThreadArgs args[5];
    int sleep_times[5] = {1, 2, 3, 4, 5}; // Ejemplo de tiempos de sueño

    // Crear los hilos
    for (int i = 0; i < 5; i++) {
        args[i].sleep_time = sleep_times[i];
        pthread_create(&hilos[i], NULL, Alarm, &args[i]);
    }

    // Crear el hilo para imprimir el estado de la bandera
    pthread_t flag_status_thread;
    pthread_create(&flag_status_thread, NULL, PrintFlagStatus, NULL);

    // Cancelar el hilo 3 (índice 2)
    Defuse(hilos[2]);

    // Reinicio hilo 5
    RestartAlarm(&hilos[4], &args[4]);

    // Esperar a que los hilos terminen
    for (int i = 0; i < 5; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Cancelar el hilo de estado de la bandera
    pthread_cancel(flag_status_thread);
    pthread_join(flag_status_thread, NULL);

    return 0;
}