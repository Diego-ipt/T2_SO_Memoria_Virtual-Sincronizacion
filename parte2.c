#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PaginaVirtual.h"
#include "algoritmos_reemplazo.h"

void leer_referencias(const char* filename, int** referencias, int* num_referencias) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    int capacity = 10;
    *referencias = (int*)malloc(capacity * sizeof(int));
    *num_referencias = 0;

    while (fscanf(file, "%d", &(*referencias)[*num_referencias]) != EOF) {
        (*num_referencias)++;
        if (*num_referencias >= capacity) {
            capacity *= 2;
            *referencias = (int*)realloc(*referencias, capacity * sizeof(int));
        }
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Uso: %s -m <num_marcos> -a <algoritmo> -f <archivo_referencias>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_marcos = 0;
    char* algoritmo = NULL;
    char* archivo_referencias = NULL;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-m") == 0) {
            num_marcos = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-a") == 0) {
            algoritmo = argv[i + 1];
        } else if (strcmp(argv[i], "-f") == 0) {
            archivo_referencias = argv[i + 1];
        } else {
            fprintf(stderr, "Parametro desconocido: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    int* referencias = NULL;
    int num_referencias = 0;
    leer_referencias(archivo_referencias, &referencias, &num_referencias);

    Page* table[SIZE] = { NULL };
    for (int i = 0; i < num_referencias; i++) {
        int marco_azar = rand() % 999; // Generar un número de marco al azar
        //se cargan las paginas en la tabla hash (supuesto de que estan en memoria, es decir, existen)
        insertar_pagina(table, referencias[i], marco_azar, 1, 0, 0);
    }
    int fallos_pagina = 0;

    if (strcmp(algoritmo, "OPTIMO") == 0) {
        printf("Algoritmo OPTIMO\n");
        fallos_pagina = reemplazo_optimo(table, num_marcos, referencias, num_referencias);
    } else if (strcmp(algoritmo, "FIFO") == 0) {
        printf("Algoritmo FIFO\n");
        fallos_pagina = reemplazo_fifo(table, num_marcos, referencias, num_referencias);
    } else if (strcmp(algoritmo, "LRU") == 0) {
        printf("Algoritmo LRU\n");
        fallos_pagina = reemplazo_lru(table, num_marcos, referencias, num_referencias);
    } else if (strcmp(algoritmo, "LRU_R") == 0) {
        printf("Algoritmo LRU_RELOJ\n");
        fallos_pagina = reemplazo_lru_reloj(table, num_marcos, referencias, num_referencias);
    } else {
        fprintf(stderr, "Algoritmo desconocido: %s\n", algoritmo);
        fprintf(stderr, "Algoritmos validos: OPTIMO, FIFO, LRU, LRU_R\n LRU_R: LRU con Reloj\n");
        return EXIT_FAILURE;
    }

    printf("Numero de fallos de pagina: %d\n", fallos_pagina);

    free(referencias);
    free_table(table, SIZE);

    return EXIT_SUCCESS;
}