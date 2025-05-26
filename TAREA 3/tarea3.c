// TAREA 3 
// INTEGRANTES :- GASPAR ALBORNOZ 
//              - RAMON ESPINOZA 
// PARALELO: ICD

#include <stdio.h>
#include <stdlib.h>
#include "tdas/list.h"
#include "tdas/extra.h"
#include <string.h>

typedef struct { 
    char nombre[50];
    int peso;
    int valor;
} Item; // estructura de los items

typedef struct Escenario{
    int id;
    char nombre[50];
    char descripcion[300];
    List *items;
    int id_conexiones[4]; // 0: arriba, 1: abajo, 2: izquierda, 3: derecha
    struct Escenario *conexiones[4];
    int esFinal;
} Escenario; // grafo y conexiones de escenario 

typedef struct {
    List *inventario;  // Lista de items recolectados
    int peso_actual;
    int puntaje;
    int tiempo_restante;
    Escenario *posicion_actual;
} Jugador; // estructura de jugador 

void leer_escenarios(List *lista_escenarios) {
    FILE *archivo = fopen("graphquest.csv", "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    // Leer encabezados del CSV
    char **campos;
    campos = leer_linea_csv(archivo, ',');
    if (campos == NULL) {
        fclose(archivo);
        return;
    }

    // Leer cada línea del archivo
    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        Escenario *aux = (Escenario *)malloc(sizeof(Escenario));
        aux->id = atoi(campos[0]);

        // Copiar nombre y descripción
        strncpy(aux->nombre, campos[1], sizeof(aux->nombre)-1);
        aux->nombre[sizeof(aux->nombre)-1] = '\0';
        strncpy(aux->descripcion, campos[2], sizeof(aux->descripcion)-1);
        aux->descripcion[sizeof(aux->descripcion)-1] = '\0';

        // Leer y procesar los ítems si hay
        if (strlen(campos[3]) > 0) {
            aux->items = list_create();
            List *lista_items_str = split_string(campos[3], ";");

            for (char *item_str = list_first(lista_items_str); item_str != NULL; item_str = list_next(lista_items_str)) {
                List *values = split_string(item_str, ",");
                Item *item_aux = (Item*)malloc(sizeof(Item));

                strcpy(item_aux->nombre, list_first(values));
                item_aux->valor = atoi(list_next(values));
                item_aux->peso = atoi(list_next(values));

                list_pushBack(aux->items, item_aux);

                list_clean(values);
                free(values);
            }

            list_clean(lista_items_str);
            free(lista_items_str);
        } else {
            aux->items = NULL;
        }

        // Leer conexiones por ID
        aux->id_conexiones[0] = atoi(campos[4]);
        aux->id_conexiones[1] = atoi(campos[5]);
        aux->id_conexiones[2] = atoi(campos[6]);
        aux->id_conexiones[3] = atoi(campos[7]);

        // Verificar si es escenario final
        if (strcmp(campos[8], "Si") == 0) {
            aux->esFinal = 1;
        } else {
            aux->esFinal = 0;
        }

        list_pushBack(lista_escenarios, aux);
    }

    fclose(archivo);

    // Asignar punteros de conexión usando los IDs
    int size = list_size(lista_escenarios);
    for (int i = 0; i < size; i++) {
        Escenario *esc = list_get(lista_escenarios, i);
        for (int j = 0; j < 4; j++) {
            int id = esc->id_conexiones[j];
            esc->conexiones[j] = NULL;
            if (id == -1) continue;

            // Buscar escenario con ese ID para enlazar
            for (int k = 0; k < size; k++) {
                Escenario *otro = list_get(lista_escenarios, k);
                if (otro->id == id) {
                    esc->conexiones[j] = otro;
                    break;
                }
            }
        }
    }
}

void mostrar_estado(Jugador *jugador) {  // Muestra el estado actual del jugador en el laberinto
    Escenario *esc = jugador->posicion_actual;  // Escenario donde está el jugador
    printf("\n=== Estado Actual ===\n");
    printf("Escenario: %s\n", esc->nombre);
    printf("Descripción: %s\n", esc->descripcion);
    printf("Tiempo restante: %d\n\n", jugador->tiempo_restante);

    // Mostrar ítems disponibles en el escenario actual
    printf("Ítems disponibles en este escenario:\n");
    if (esc->items == NULL || list_size(esc->items) == 0) {
        printf("No hay ítems disponibles aquí\n\n");
    } else {
        int i = 1;
        Item *it = list_first(esc->items);
        while (it != NULL) {
            printf("  %d) %s (Peso: %d, Valor: %d)\n\n", i, it->nombre, it->peso, it->valor);
            it = list_next(esc->items);
            i++;
        }
    }

    // Mostrar inventario del jugador con peso y puntaje acumulado
    printf("Inventario del jugador:\n");
    if (jugador->inventario == NULL || list_size(jugador->inventario) == 0) {
        printf("Inventario vacío\n\n");
    } else {
        int total_peso = 0;
        int total_valor = 0;
        int i = 1;
        Item *it = list_first(jugador->inventario);
        while (it != NULL) {
            printf("  %d) %s (Peso: %d, Valor: %d)\n", i, it->nombre, it->peso, it->valor);
            total_peso += it->peso;
            total_valor += it->valor;
            it = list_next(jugador->inventario);
            i++;
        }
        printf("Peso total: %d\n", total_peso);
        printf("Puntaje acumulado: %d\n\n", total_valor);
    }

    // Mostrar direcciones disponibles para moverse desde el escenario actual
    printf("Direcciones disponibles:\n");
    char *direcciones[] = {"Arriba", "Abajo", "Izquierda", "Derecha"};
    for (int i = 0; i < 4; i++) {
        if (esc->conexiones[i] != NULL)
            printf("%s\n", direcciones[i]);
    }
}

int avanzar(Jugador *jugador) {  // Permite al jugador avanzar a otro escenario y actualiza el estado
    Escenario *esc = jugador->posicion_actual;
    printf("Seleccione dirección para avanzar:\n");
    char *direcciones[] = {"Arriba", "Abajo", "Izquierda", "Derecha"};
    int opciones_validas[4] = {0, 0, 0, 0}; // Guarda índices de direcciones válidas
    int contador_opc = 0;
    int i = 0;

    // Mostrar solo las direcciones disponibles para avanzar
    while (i < 4) {
        if (esc->conexiones[i] != NULL) {
            contador_opc++;
            printf("  %d) %s\n", contador_opc, direcciones[i]);
            opciones_validas[contador_opc - 1] = i; // Guardar dirección válida
        }
        i++;
    }

    // Si no hay direcciones disponibles, no se puede avanzar
    if (contador_opc == 0) {
        printf("No hay direcciones disponibles para avanzar.\n");
        return 0; // No termina el juego, pero no avanza
    }

    int opcion;
    scanf("%d", &opcion);
    if (opcion < 1 || opcion > contador_opc) {  // Validar opción ingresada
        printf("Opción inválida.\n");
        return 0;
    }

    int dir = opciones_validas[opcion - 1]; // Obtener la dirección seleccionada
    Escenario *nuevo_esc = esc->conexiones[dir];
    jugador->posicion_actual = nuevo_esc;   // Actualizar posición del jugador

    // Calcular tiempo gastado según peso actual y restarlo al tiempo restante
    int tiempo_gastado = (int)((jugador->peso_actual + 1) / 10.0);
    jugador->tiempo_restante -= tiempo_gastado;

    // Verificar si se llegó al escenario final
    if (nuevo_esc->esFinal) {
        printf("¡Has llegado al escenario final!\n");
        printf("Inventario final y puntaje:\n");
        if (jugador->inventario == NULL || list_size(jugador->inventario) == 0) {
            printf("  (Inventario vacío)\n");
            printf("Peso total: %d KG\n", 0);
            printf("Puntaje final: %d\n", 0);
        } else {
            int total_peso = 0;
            int total_valor = 0;
            int i = 1;
            Item *it = list_first(jugador->inventario);
            while (it != NULL) {
                printf("  %d) %s (Peso: %d, Valor: %d)\n", i, it->nombre, it->peso, it->valor);
                total_peso += it->peso;
                total_valor += it->valor;
                it = list_next(jugador->inventario);
                i++;
            }
            printf("Peso total: %d KG\n", total_peso);
            printf("Puntaje final: %d\n", total_valor);
        }
        return 1; // Juego terminado (llegó al final)
    }

    printf("Avanzaste a %s. Tiempo usado: %d\n", nuevo_esc->nombre, tiempo_gastado);

    mostrar_estado(jugador); // Mostrar el estado actualizado del jugador

    // Verificar si se acabó el tiempo
    if (jugador->tiempo_restante <= 0) {
        printf("Se agotó el tiempo. Has perdido.\n");
        return 1; // Juego terminado (tiempo agotado)
    }
    return 0; // Juego continúa
}

void recoger_items(Jugador *jugador) {  // Recoge un ítem del escenario actual y lo añade al inventario
    Escenario *esc_aux = jugador->posicion_actual;

    // Verificar si hay ítems disponibles para recoger
    if (esc_aux->items == NULL || list_size(esc_aux->items) == 0) {
        printf("No hay ítems para recoger en este escenario.\n");
        return;
    }

    // Mostrar los ítems disponibles
    printf("Ítems disponibles para recoger:\n");
    int i = 1;
    Item *item_aux = list_first(esc_aux->items);
    while (item_aux != NULL) {
        printf("  %d) %s (Peso: %d, Valor: %d)\n", i, item_aux->nombre, item_aux->peso, item_aux->valor);
        item_aux = list_next(esc_aux->items);
        i++;
    }

    // Pedir al jugador que seleccione un ítem o cancele
    printf("Ingrese el número del ítem que desea recoger (0 para cancelar): ");
    int opcion;
    scanf("%d", &opcion);

    // Validar opción ingresada
    if (opcion <= 0 || opcion > list_size(esc_aux->items)) {
        printf("Operación cancelada o opción inválida.\n");
        return;
    }

    // Mover puntero a ítem seleccionado
    item_aux = list_first(esc_aux->items);
    for (int j = 1; j < opcion; j++) {
        item_aux = list_next(esc_aux->items);
    }

    // Crear una copia del ítem y agregarla al inventario
    Item *nuevo = (Item *)malloc(sizeof(Item));
    strcpy(nuevo->nombre, item_aux->nombre);
    nuevo->peso = item_aux->peso;
    nuevo->valor = item_aux->valor;

    list_pushBack(jugador->inventario, nuevo);
    jugador->peso_actual += nuevo->peso;     // Actualizar peso total del jugador
    jugador->puntaje += nuevo->valor;        // Actualizar puntaje acumulado
    jugador->tiempo_restante--;               // Restar tiempo por acción

    // Eliminar el ítem del escenario para que no pueda recogerse nuevamente
    list_popIndex(esc_aux->items, opcion - 1);

    printf("Recogiste el ítem '%s'.\n", nuevo->nombre);
}

void descartar_items(Jugador *jugador) {  // Elimina un ítem del inventario del jugador y actualiza su puntaje y peso
    // Verificar si el inventario está vacío
    if (jugador->inventario == NULL || list_size(jugador->inventario) == 0) {
        printf("No tienes ítems en tu inventario.\n");
        return;
    }

    // Mostrar los ítems en el inventario
    printf("Ítems en tu inventario:\n");
    int i = 1;
    Item *item_aux = list_first(jugador->inventario);
    while (item_aux != NULL) {
        printf("  %d) %s (Peso: %d, Valor: %d)\n", i, item_aux->nombre, item_aux->peso, item_aux->valor);
        item_aux = list_next(jugador->inventario);
        i++;
    }

    // Pedir al jugador que seleccione un ítem para descartar o cancelar
    printf("Ingrese el número del ítem que desea descartar (0 para cancelar): ");
    int opcion;
    scanf("%d", &opcion);

    // Validar la opción ingresada
    if (opcion <= 0 || opcion > list_size(jugador->inventario)) {
        printf("Operación cancelada o opción inválida.\n");
        return;
    }
    
    // Mover puntero al ítem seleccionado
    item_aux = list_first(jugador->inventario);
    for (int j = 1; j < opcion; j++) {
        item_aux = list_next(jugador->inventario);
    }

    // Actualizar peso, puntaje y tiempo tras descartar
    jugador->peso_actual -= item_aux->peso;
    jugador->puntaje -= item_aux->valor;
    jugador->tiempo_restante--;

    printf("Descartaste el ítem '%s'.\n", item_aux->nombre);

    // Remover el ítem del inventario
    list_popIndex(jugador->inventario, opcion - 1);
}

void iniciar_juego_1_jugador(List *lista_escenarios) {  // Inicia el modo de juego para un solo jugador
    Jugador jugador;  
    jugador.inventario = list_create();                  // Crear lista para inventario
    jugador.posicion_actual = list_first(lista_escenarios);  // Posición inicial en el primer escenario
    jugador.peso_actual = 0;
    jugador.puntaje = 0;
    jugador.tiempo_restante = 10;                         // Tiempo inicial disponible

    int fin = 0;

    do {  // Ciclo principal del juego
        printf("\n--- Turno del Jugador ---\n");
        mostrar_estado(&jugador);                         // Mostrar estado actual del jugador

        // Menú de opciones
        printf("\nOpciones:\n");
        printf("1) Recoger Ítem(s)\n");
        printf("2) Descarta Ítem(s)\n");
        printf("3) Avanzar en una Dirección\n");
        printf("4) Reiniciar Partida\n");
        printf("5) Salir del Juego\n");
        printf("Ingrese opción: ");

        char opcion;
        scanf(" %c", &opcion);

        switch (opcion) {
            case '1':
                recoger_items(&jugador);                   // Recoger ítems del escenario actual
                break;
            case '2':
                descartar_items(&jugador);                 // Descartar ítems del inventario
                break;
            case '3':
                fin = avanzar(&jugador);                    // Avanzar a un nuevo escenario; puede terminar el juego
                break;
            case '4':                                      // Reiniciar partida
                list_clean(jugador.inventario);
                jugador.posicion_actual = list_first(lista_escenarios);
                jugador.peso_actual = 0;
                jugador.puntaje = 0;
                jugador.tiempo_restante = 10;
                printf("Partida reiniciada.\n");
                break;
            case '5':
                fin = 1;                                    // Salir del juego
                break;
            default:
                printf("Opción inválida.\n");
        }

        // Verificar condiciones de fin: llegar al escenario final o quedarse sin tiempo
        if (jugador.posicion_actual->esFinal || jugador.tiempo_restante <= 0)
            fin = 1;

        presioneTeclaParaContinuar();
        limpiarPantalla();

    } while (!fin);

    // Liberar memoria del inventario
    while (list_size(jugador.inventario) > 0) {
        Item *it = list_first(jugador.inventario);
        list_popFront(jugador.inventario);
        free(it);
    }
    list_clean(jugador.inventario);
}

int avanzar_dos_jugadores(Jugador *jugador1, Jugador *jugador2) { // funacion que compara la situacion de termino en 2 jugadores
    int fin_jugador1 = 0; 
    int fin_jugador2 = 0;

    if (!(jugador1->tiempo_restante <= 0 || jugador1->posicion_actual->esFinal)) {
        printf("\nTurno de Jugador 1 para avanzar:\n");
        fin_jugador1 = avanzar(jugador1);
    } else {
        printf("Jugador 1 ya terminó (sin tiempo o llegó a la salida).\n");
        fin_jugador1 = 1;
    }

    if (!(jugador2->tiempo_restante <= 0 || jugador2->posicion_actual->esFinal)) {
        printf("\nTurno de Jugador 2 para avanzar:\n");
        fin_jugador2 = avanzar(jugador2);
    } else {
        printf("Jugador 2 ya terminó (sin tiempo o llegó a la salida).\n");
        fin_jugador2 = 1;
    }

    if (fin_jugador1 && fin_jugador2) {
        printf("\n--- Ambos jugadores han terminado la partida ---\n");
        return 1;
    }
    return 0;
}

void iniciar_juego_dos_jugadores(List *lista_escenarios) {
    // Inicializar ambos jugadores con inventario vacío, posición inicial, peso, puntaje y tiempo
    Jugador jugador1, jugador2;
    jugador1.inventario = list_create();
    jugador1.posicion_actual = list_first(lista_escenarios);
    jugador1.peso_actual = 0;
    jugador1.puntaje = 0;
    jugador1.tiempo_restante = 10;

    jugador2.inventario = list_create();
    jugador2.posicion_actual = list_first(lista_escenarios);
    jugador2.peso_actual = 0;
    jugador2.puntaje = 0;
    jugador2.tiempo_restante = 10;

    int fin = 0;
    int turno = 1;

    // Ciclo principal del juego que continúa hasta que ambos jugadores terminen o decidan salir
    while (!fin) {
        // Verificar si cada jugador ya terminó por tiempo o llegó al escenario final
        int jugador1_terminado = (jugador1.tiempo_restante <= 0 || jugador1.posicion_actual->esFinal);
        int jugador2_terminado = (jugador2.tiempo_restante <= 0 || jugador2.posicion_actual->esFinal);

        // Si ambos terminaron, mostrar resumen final con inventarios y puntajes, luego terminar el juego
        if (jugador1_terminado && jugador2_terminado) {
            printf("\n--- Ambos jugadores han llegado a la salida ---\n");
            // Mostrar inventario y puntaje del jugador 1
            // Mostrar inventario y puntaje del jugador 2
            // Sumar y mostrar puntaje total combinado
            break;
        }

        // Determinar qué jugador tiene el turno actual
        Jugador *jugador_actual = (turno == 1) ? &jugador1 : &jugador2;

        // Si el jugador actual ya terminó, saltar su turno
        if (jugador_actual->tiempo_restante <= 0 || jugador_actual->posicion_actual->esFinal) {
            printf("El Jugador %d ya terminó (sin tiempo o llegó a la salida).\n", turno);
            turno = (turno == 1) ? 2 : 1;
            continue;
        }

        // Mostrar estado del jugador actual y ofrecer dos acciones por turno
        printf("\n--- Turno del Jugador %d ---\n", turno);
        mostrar_estado(jugador_actual);

        for (int accion = 1; accion <= 2; accion++) {
            // Menú de acciones para el jugador en cada una de sus dos acciones permitidas
            printf("\nAcción %d:\n", accion);
            printf("1) Recoger Ítem(s)\n");
            printf("2) Descarta Ítem(s)\n");
            printf("3) Avanzar en una Dirección\n");
            printf("4) Pasar\n");
            printf("5) Reiniciar Partida\n");
            printf("6) Salir del Juego\n");
            printf("Ingrese opción: ");

            char opcion;
            scanf(" %c", &opcion);

            // Ejecutar acción según opción elegida
            if (opcion == '1') {
                recoger_items(jugador_actual);
            } else if (opcion == '2') {
                descartar_items(jugador_actual);
            } else if (opcion == '3') {
                int fin_avance = avanzar(jugador_actual);
                // Verificar condiciones para terminar el juego tras avance
                if (fin_avance && (jugador1_terminado && jugador2_terminado)) {
                    fin = 1;
                    break;
                }
            } else if (opcion == '4') {
                printf("Jugador %d pasó su acción.\n", turno);
            } else if (opcion == '5') {
                // Reiniciar partida para ambos jugadores y volver al menú principal
                list_clean(jugador1.inventario);
                jugador1.posicion_actual = list_first(lista_escenarios);
                jugador1.peso_actual = 0;
                jugador1.puntaje = 0;
                jugador1.tiempo_restante = 10;

                list_clean(jugador2.inventario);
                jugador2.posicion_actual = list_first(lista_escenarios);
                jugador2.peso_actual = 0;
                jugador2.puntaje = 0;
                jugador2.tiempo_restante = 10;

                printf("Partida reiniciada. Volviendo al menú principal.\n");
                return;
            } else if (opcion == '6') {
                // Salir del juego
                fin = 1;
                break;
            } else {
                // Opción inválida: repetir la acción actual
                printf("Opción inválida.\n");
                accion--;
            }

            if (fin) break;
        }

        // Cambiar turno entre jugador 1 y jugador 2
        if (!fin) {
            turno = (turno == 1) ? 2 : 1;
        }

        presioneTeclaParaContinuar();
        limpiarPantalla();
    }

    // Liberar memoria y limpiar inventarios de ambos jugadores al finalizar el juego
    while (list_size(jugador1.inventario) > 0) {
        Item *it = list_first(jugador1.inventario);
        list_popFront(jugador1.inventario);
        free(it);
    }
    list_clean(jugador1.inventario);

    while (list_size(jugador2.inventario) > 0) {
        Item *it = list_first(jugador2.inventario);
        list_popFront(jugador2.inventario);
        free(it);
    }
    list_clean(jugador2.inventario);
}
void liberar_escenarios(List *escenarios) { // libera los escenarios de los duplicados
    int size = list_size(escenarios);
    for (int i = 0; i < size; i++) {
        Escenario *esc = list_get(escenarios, i);

        // Liberar todos los ítems del escenario, si existen
        if (esc->items != NULL) {
            while (list_size(esc->items) > 0) {
                Item *it = list_first(esc->items);  
                list_popFront(esc->items);          
                free(it);                           
            }
            list_clean(esc->items);  
            free(esc->items);        
        }

        free(esc);  
    }

    list_clean(escenarios); 
    free(escenarios);        
}

List *clonar_escenarios(List *original) {
    int size = list_size(original);
    List *copia = list_create();  // Crear nueva lista para guardar los escenarios clonados

    // Array temporal para guardar los escenarios clonados, necesario para establecer conexiones más tarde
    Escenario **clonados = malloc(sizeof(Escenario*) * size);

    // Primer paso: clonar todos los escenarios sin conexiones aún
    for (int i = 0; i < size; i++) {
        Escenario *esc_orig = list_get(original, i);
        Escenario *esc_copia = malloc(sizeof(Escenario));

        // Copiar campos
        esc_copia->id = esc_orig->id;
        strcpy(esc_copia->nombre, esc_orig->nombre);
        strcpy(esc_copia->descripcion, esc_orig->descripcion);
        esc_copia->esFinal = esc_orig->esFinal;

        // Clonar lista de ítems
        if (esc_orig->items == NULL || list_size(esc_orig->items) == 0) {
            esc_copia->items = list_create();  // Crear lista vacía si no hay ítems
        } else {
            esc_copia->items = list_create();  // Crear lista para ítems clonados
            Item *item_orig = list_first(esc_orig->items);
            while (item_orig != NULL) {
                // Clonar cada ítem uno por uno
                Item *item_copia = malloc(sizeof(Item));
                strcpy(item_copia->nombre, item_orig->nombre);
                item_copia->peso = item_orig->peso;
                item_copia->valor = item_orig->valor;
                list_pushBack(esc_copia->items, item_copia);  
                item_orig = list_next(esc_orig->items);       
            }
        }

        // Copiar ids de conexiones (por ahora solo los ids)
        for (int j = 0; j < 4; j++) {
            esc_copia->id_conexiones[j] = esc_orig->id_conexiones[j];
            esc_copia->conexiones[j] = NULL;  
        }

        clonados[i] = esc_copia;           
        list_pushBack(copia, esc_copia);   
    }

    // Segundo paso: establecer las conexiones reales entre escenarios clonados
    for (int i = 0; i < size; i++) {
        Escenario *esc_copia = clonados[i];
        for (int j = 0; j < 4; j++) {
            int id_conn = esc_copia->id_conexiones[j];
            esc_copia->conexiones[j] = NULL;
            if (id_conn == -1) continue; 

            // Buscar el clon que tenga el id correspondiente y enlazarlo
            for (int k = 0; k < size; k++) {
                if (clonados[k]->id == id_conn) {
                    esc_copia->conexiones[j] = clonados[k];
                    break;
                }
            }
        }
    }

    free(clonados);  
    return copia;    
}

int main() {
    List *lista_escenarios_original = list_create();  // Lista original cargada desde archivo
    List *lista_escenarios_juego = NULL;              // Clon usado durante cada partida
    int escenarios_cargados = 0;            
    char opcion;

    do {
        // Menú principal
        puts("========================================");
        puts("     Menú Principal");
        puts("========================================");
        puts("1) Cargar Laberinto");
        puts("2) Iniciar Partida Solo");
        puts("3) Iniciar Partida Multijugador");
        puts("4) Salir");
        printf("Ingrese su opción: ");
        scanf(" %c", &opcion);

        switch (opcion) {
            case '1':
                if (!escenarios_cargados) {
                    leer_escenarios(lista_escenarios_original);  // Carga desde CSV
                    if (list_size(lista_escenarios_original) > 0) {
                        escenarios_cargados = 1;
                        printf("Laberinto cargado exitosamente.\n");
                    } else {
                        printf("No se pudo cargar el laberinto.\n");
                    }
                } else {
                    printf("El laberinto ya fue cargado.\n");
                }
                break;

            case '2':
                if (escenarios_cargados) {
                    if (lista_escenarios_juego != NULL) {
                        liberar_escenarios(lista_escenarios_juego);  // Limpia memoria previa
                    }
                    lista_escenarios_juego = clonar_escenarios(lista_escenarios_original);  // Clona el laberinto
                    iniciar_juego_1_jugador(lista_escenarios_juego);                        // Ejecuta partida
                    liberar_escenarios(lista_escenarios_juego);                             // Libera recursos
                    lista_escenarios_juego = NULL;
                } else {
                    printf("Debe cargar el laberinto primero.\n");
                }
                break;

            case '3':
                if (escenarios_cargados) {
                    if (lista_escenarios_juego != NULL) {
                        liberar_escenarios(lista_escenarios_juego);
                    }
                    lista_escenarios_juego = clonar_escenarios(lista_escenarios_original);
                    iniciar_juego_dos_jugadores(lista_escenarios_juego);
                    liberar_escenarios(lista_escenarios_juego);
                    lista_escenarios_juego = NULL;
                } else {
                    printf("Debe cargar el laberinto primero.\n");
                }
                break;
        }

        presioneTeclaParaContinuar();  
        limpiarPantalla();             

    } while (opcion != '4');

    liberar_escenarios(lista_escenarios_original);  // Libera memoria 
    return 0;
}
