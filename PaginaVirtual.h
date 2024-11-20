#ifndef PAGINAVIRTUAL_H
#define PAGINAVIRTUAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 15 //numero impar

//Estructura de lista enlazada
typedef struct Page {
    int num_pagina;         //clave --valor único
    int num_marco;          //valor
    int valido;            // Bit de válido (1: en memoria, 0: no en memoria)
    int modificado;        // Bit de modificado (1: modificado, 0: no modificado)
    int referencia;      // Bit de uso reciente o lectura (de referencia) (1: usado, 0: no usado)
    struct Page* next;
} Page;


// funcion hash para calcular indice
int hash_function(int num_pagina) {
    return num_pagina % SIZE;
}


// Crear una nueva entrada en la tabla de páginas
Page* crear_pagina(int num_pagina, int num_marco, int valido, int modificado, int uso_reciente) {
    Page* new_page = (Page*)malloc(sizeof(Page));
    new_page->num_pagina = num_pagina;
    new_page->num_marco = num_marco;
    new_page->valido = valido;
    new_page->modificado = modificado;
    new_page->referencia = uso_reciente;
    new_page->next = NULL;
    return new_page;
}


// Insertar una página en la tabla hash
void insertar_pagina(Page* table[], int num_pagina, int num_marco, int valido, int modificado, int uso_reciente) {
    int index = hash_function(num_pagina);
    Page* current = table[index];
    
    // Buscar si la página ya existe
    while (current != NULL) {
        if (current->num_pagina == num_pagina) {
            // Si la página ya existe, modificar sus valores
            current->num_marco = num_marco;
            current->valido = valido;
            current->modificado = modificado;
            current->referencia = uso_reciente;
            return;
        }
        current = current->next;
    }
    
    // Si la página no existe, crear una nueva y agregarla a la tabla
    Page* new_page = crear_pagina(num_pagina, num_marco, valido, modificado, uso_reciente);
    new_page->next = table[index];
    table[index] = new_page;
}

// Buscar una página en la tabla hash
Page* buscar_pagina(Page* table[], int num_pagina) {
    int index = hash_function(num_pagina);
    Page* current = table[index];
    while (current != NULL) { //recorrer la lista hasta encontrar el nodo correcto
        if (current->num_pagina == num_pagina) { 
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Liberar la memoria de la tabla hash
void free_table(Page* table[], int size) {
    for (int i = 0; i < SIZE; i++) {
        Page* current = table[i];
        while (current != NULL) {
            Page* temp = current;
            current = current->next;
            free(temp);
        }
    }
}

#endif 
