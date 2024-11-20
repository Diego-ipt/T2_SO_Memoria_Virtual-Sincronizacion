#ifndef ALGORITMOS_REEMPLAZO_H
#define ALGORITMOS_REEMPLAZO_H

#include "PaginaVirtual.h"

// Prototipos de funciones
int reemplazo_optimo(Page* table[], int num_marcos, int referencias[], int num_referencias);
int reemplazo_fifo(Page* table[], int num_marcos, int referencias[], int num_referencias);
int reemplazo_lru(Page* table[], int num_marcos, int referencias[], int num_referencias);
int reemplazo_lru_reloj(Page* table[], int num_marcos, int referencias[], int num_referencias);

int reemplazo_optimo(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    int marcos[num_marcos];
    int num_marcos_ocupados = 0;

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        Page* pagina = buscar_pagina(table, pagina_actual);

        if (pagina == NULL || !pagina->valido) {
            fallos_pagina++;
            if (num_marcos_ocupados < num_marcos) {
                marcos[num_marcos_ocupados++] = pagina_actual;
            } else {
                int farthest = i + 1;
                int replace_index = -1;
                for (int j = 0; j < num_marcos; j++) {
                    int k;
                    for (k = i + 1; k < num_referencias; k++) {
                        if (referencias[k] == marcos[j]) {
                            if (k > farthest) {
                                farthest = k;
                                replace_index = j;
                            }
                            break;
                        }
                    }
                    if (k == num_referencias) {
                        replace_index = j;
                        break;
                    }
                }
                if (replace_index == -1) {
                    replace_index = 0;
                }
                marcos[replace_index] = pagina_actual;
            }
            insertar_pagina(table, pagina_actual, 0, 1, 0, 0);
        }
    }
    return fallos_pagina;
}

int reemplazo_fifo(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    int marcos[num_marcos];
    int front = 0, rear = 0, num_marcos_ocupados = 0;

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        Page* pagina = buscar_pagina(table, pagina_actual);

        if (pagina == NULL || !pagina->valido) {
            fallos_pagina++;
            if (num_marcos_ocupados < num_marcos) {
                marcos[rear] = pagina_actual;
                rear = (rear + 1) % num_marcos;
                num_marcos_ocupados++;
            } else {
                marcos[front] = pagina_actual;
                front = (front + 1) % num_marcos;
                rear = (rear + 1) % num_marcos;
            }
            insertar_pagina(table, pagina_actual, 0, 1, 0, 0);
        }
    }
    return fallos_pagina;
}


//falta revisar a fondo
int reemplazo_lru(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    int marcos[num_marcos];
    int contador[num_marcos];
    int num_marcos_ocupados = 0;

    // Inicializar contadores
    for (int i = 0; i < num_marcos; i++) {
        contador[i] = 0;
    }

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        Page* pagina = buscar_pagina(table, pagina_actual);

        // Incrementar contadores periódicamente
        for (int j = 0; j < num_marcos; j++) {
            if (contador[j] != -1 && contador[j] != 1) { // Solo incrementar contadores válidos y no en 1
                contador[j]++;
            } else if (contador[j] == 1) {
                contador[j] = 0; // Resetear contador de 1 a 0
            }
        }

        if (pagina == NULL || !pagina->valido) {
            fallos_pagina++;
            if (num_marcos_ocupados < num_marcos) {
                marcos[num_marcos_ocupados] = pagina_actual;
                contador[num_marcos_ocupados] = 0; // Se ha usado recientemente
                num_marcos_ocupados++;
            } else {
                int lru_index = 0;
                for (int j = 1; j < num_marcos; j++) {
                    if (contador[j] > contador[lru_index]) {
                        lru_index = j;
                    }
                }
                marcos[lru_index] = pagina_actual;
                contador[lru_index] = 0; // Se ha usado recientemente
            }
            insertar_pagina(table, pagina_actual, 0, 1, 0, 0);
        } else {
            for (int j = 0; j < num_marcos; j++) {
                if (marcos[j] == pagina_actual) {
                    contador[j] = 0; // Se ha usado recientemente
                    break;
                }
            }
        }
    }
    return fallos_pagina;
}


//falta revisar a fondo
int reemplazo_lru_reloj(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    int marcos[num_marcos];
    int uso[num_marcos];
    int puntero = 0;
    int num_marcos_ocupados = 0;

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        Page* pagina = buscar_pagina(table, pagina_actual);

        if (pagina == NULL || !pagina->valido) {
            fallos_pagina++;
            while (num_marcos_ocupados >= num_marcos && uso[puntero] == 1) {
                uso[puntero] = 0;
                puntero = (puntero + 1) % num_marcos;
            }
            if (num_marcos_ocupados < num_marcos) {
                marcos[puntero] = pagina_actual;
                uso[puntero] = 1;
                num_marcos_ocupados++;
            } else {
                marcos[puntero] = pagina_actual;
                uso[puntero] = 1;
                puntero = (puntero + 1) % num_marcos;
            }
            insertar_pagina(table, pagina_actual, 0, 1, 0, 0);
        } else {
            for (int j = 0; j < num_marcos; j++) {
                if (marcos[j] == pagina_actual) {
                    uso[j] = 1;
                    break;
                }
            }
        }
    }
    return fallos_pagina;
}

#endif