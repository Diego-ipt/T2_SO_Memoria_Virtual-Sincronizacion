#ifndef ALGORITMOS_REEMPLAZO_H
#define ALGORITMOS_REEMPLAZO_H

#include "PaginaVirtual.h"
#include <stdbool.h>

// Prototipos de funciones
int reemplazo_optimo(Page* table[], int num_marcos, int referencias[], int num_referencias);
int reemplazo_fifo(Page* table[], int num_marcos, int referencias[], int num_referencias);
int reemplazo_lru(Page* table[], int num_marcos, int referencias[], int num_referencias);
int reemplazo_lru_reloj(Page* table[], int num_marcos, int referencias[], int num_referencias);

int reemplazo_optimo(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    Page marcos[num_marcos];
    bool acierto = false;

    for (int i = 0; i < num_marcos; i++) {
        marcos[i].valido = false; // Inicializar los marcos con un valor que no sea una página válida
        marcos[i].num_pagina = -1;
        //no hay nada cargado aun
    }

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        acierto = false;

        // Verificar si la página está en los marcos
        for (int j = 0; j < num_marcos; j++) {
            if (marcos[j].num_pagina == pagina_actual && marcos[j].valido) {
                acierto = true;
                break;
            }
        }

        if (!acierto) {
            fallos_pagina++;
            // Buscar en la tabla hash, acceso a tabla de paginas
            Page* pagina = buscar_pagina(table, pagina_actual);

            int replace_index = -1;
            int farthest = i + 1;
            for (int j = 0; j < num_marcos; j++) {
                int k;
                for (k = i + 1; k < num_referencias; k++) {
                    if (marcos[j].num_pagina == referencias[k]) {
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
            if (replace_index == -1) {//no hay un candidato mejor
                replace_index = 0;
            }
            marcos[replace_index].num_pagina = pagina_actual;
            marcos[replace_index].valido = true;
        }
    }
    return fallos_pagina;
}

int reemplazo_fifo(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    Page marcos[num_marcos];
    bool acierto = false;

    int front = 0;

    for (int i = 0; i < num_marcos; i++) {
        marcos[i].valido = false; // Inicializar los marcos con un valor que no sea una página válida
        marcos[i].num_pagina = -1;//no hay nada cargado aun
    }

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        acierto = false;

        // Verificar si la página está en los marcos
        for (int j = 0; j < num_marcos; j++) {
            if (marcos[j].num_pagina == pagina_actual && marcos[j].valido) {
                acierto = true;
                break;
            }
        }

        if (!acierto) {
            fallos_pagina++;
            // Buscar en la tabla hash, acceso a tabla de paginas
            Page* pagina = buscar_pagina(table, pagina_actual);

            marcos[front].num_pagina = pagina_actual;
            marcos[front].valido = true;
            front = (front + 1) % num_marcos;//se avanza con el FIFO
        }
    }
    return fallos_pagina;
}


int reemplazo_lru(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    Page marcos[num_marcos];
    bool acierto = false;

    for (int i = 0; i < num_marcos; i++) {
        marcos[i].valido = false; // Inicializar los marcos con un valor que no sea una página válida
        marcos[i].num_pagina = -1;//no hay nada cargado aun
    }

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        acierto = false;
        int pag_referenciada = -1;

        // Verificar si la página está en los marcos y actualizacion periodica
        for (int j = 0; j < num_marcos; j++) {
            if (marcos[j].num_pagina == pagina_actual && marcos[j].valido) {
                acierto = true;
                marcos[j].referencia = 0;//se ha referenciado recientemente
                pag_referenciada = j;
                break;
            }
        }
        /*Aproximación para LRU
            Muchas aproximaciones, todas usan bit R
                Mantener un contador por cada página
                    Periódicamente para cada página
                        • (1) If R == 0 then incrementar contador (no se ha usado)
                        • (2) If R == 1 then contador = 0 (se ha usado)*/
        for (int h = 0; h < num_marcos; h++) {
            if (h != pag_referenciada) {
                marcos[h].referencia = 1;//no se ha referenciado recientemente
            }
        }
        

        if (!acierto) {
            fallos_pagina++;
            // Buscar en la tabla hash, acceso a tabla de paginas
            Page* pagina = buscar_pagina(table, pagina_actual);
            int replace_index = -1;

            for (int j = 0; j < num_marcos; j++) {
                if (marcos[j].referencia == 1) {
                    replace_index = j;
                    break;
                }
            }
            if (replace_index == -1) {//no hay un candidato mejor
                replace_index = 0;
            }
            marcos[replace_index].num_pagina = pagina_actual;
            marcos[replace_index].valido = true;
        }
    }
    return fallos_pagina;
}


int reemplazo_lru_reloj(Page* table[], int num_marcos, int referencias[], int num_referencias) {
    int fallos_pagina = 0;
    Page marcos[num_marcos];
    bool acierto = false;

    int front = 0;

    for (int i = 0; i < num_marcos; i++) {
        marcos[i].valido = false; // Inicializar los marcos con un valor que no sea una página válida
        marcos[i].num_pagina = -1;//no hay nada cargado aun
    }

    for (int i = 0; i < num_referencias; i++) {
        int pagina_actual = referencias[i];
        acierto = false;
        int init_front = front;
        // Verificar si la página está en los marcos
        for (int j = 0; j < num_marcos; j++) {
            if (marcos[j].num_pagina == pagina_actual && marcos[j].valido) {
                acierto = true;
                marcos[j].referencia = 1;//se ha referenciado recientemente
                break;
            }
        }

        if (!acierto) {
            fallos_pagina++;
            // Buscar en la tabla hash, acceso a tabla de paginas
            Page* pagina = buscar_pagina(table, pagina_actual);
            int replace_index = -1;
            /* LRU con Reloj. Página no usada recientemente o Segunda Oportunidad
                Reemplazar una página que sea de las más viejas
                Organizar los marcos de páginas en una cola circular FIFO y 
                    recorrer cola buscando víctima de la siguiente manera
                         • Si bit R = 0 página es elegida para reemplazo
                         • Si bit R = 1 página ha sido usada, darle segunda oportunidad y poner bit 
                           en 0 y seguir con la siguiente página*/
            while (true) {
                if (marcos[front].referencia == 0) {
                    replace_index = front;
                    break;
                } else {
                    marcos[front].referencia = 0;
                }
                front = (front + 1) % num_marcos;
            }
            if (replace_index == -1) {//no hay un candidato mejor
                replace_index = 0;
            }
            marcos[replace_index].num_pagina = pagina_actual;
            marcos[replace_index].valido = true;
        }
    }
    return fallos_pagina;
}


#endif