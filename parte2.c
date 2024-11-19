#include "PaginaVirtual.h"

int main() {
    Page* Hashtable[SIZE] = { NULL };

    insertar_pagina(Hashtable, 1, 2, 1, 0, 0);
    insertar_pagina(Hashtable, 3, 2, 1, 0, 0);
    insertar_pagina(Hashtable, 5, 2, 1, 0, 0);

    // Buscar una página
    Page* page = buscar_pagina(Hashtable, 5);
    if (page != NULL) {
        printf("pagina encontrada %d en marco %d.\n", page->num_pagina, page->num_marco);
    } else {
        printf("pagina no encontrada.\n");
    }    

    return 0;
}