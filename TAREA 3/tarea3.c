#include <stdio.h>
#include <stdlib.h>
#include "tdas/list.h"
#include "tdas/extra.h"
#include <string.h>

typedef struct {
    char nombre[50];
    int peso;
    int valor;
} Item;

typedef struct Escenario{
    int id;
    char nombre[50];
    char descripcion[300];
    List *items;
    int id_conexiones[4]; // 0: arriba, 1: abajo, 2: izquierda, 3: derecha
    struct Escenario *conexiones[4];
    int esFinal;
} Escenario;

typedef struct {
    List *inventario;  // Lista de items recolectados
    int peso_actual;
    int puntaje;
    int tiempo_restante;
    Escenario *posicion_actual;
} Jugador;

void leer_escenarios(List *lista_escenarios) {
    FILE *archivo = fopen("graphquest.csv", "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    char **campos;
    campos = leer_linea_csv(archivo, ','); // Leer encabezados
    if (campos == NULL) {
        fclose(archivo);
        return;
    }

    int count = 0;
    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        Escenario *aux = (Escenario *)malloc(sizeof(Escenario));
        aux->id = atoi(campos[0]);
        strncpy(aux->nombre, campos[1], sizeof(aux->nombre)-1);
        aux->nombre[sizeof(aux->nombre)-1] = '\0';
        strncpy(aux->descripcion, campos[2], sizeof(aux->descripcion)-1);
        aux->descripcion[sizeof(aux->descripcion)-1] = '\0';

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

        aux->id_conexiones[0] = atoi(campos[4]);
        aux->id_conexiones[1] = atoi(campos[5]);
        aux->id_conexiones[2] = atoi(campos[6]);
        aux->id_conexiones[3] = atoi(campos[7]);

        aux->esFinal = atoi(campos[8]);

        list_pushBack(lista_escenarios, aux);
        count++;
    }
    fclose(archivo);

    int size = list_size(lista_escenarios);

    for (int i = 0; i < size; i++) {
        Escenario *esc = list_get(lista_escenarios, i);
        for (int j = 0; j < 4; j++) {
            int id = esc->id_conexiones[j];
            esc->conexiones[j] = NULL;
            if (id == -1) continue;

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

void mostrar_estado(Jugador *jugador) {
    Escenario *esc = jugador->posicion_actual;
    printf("\n=== Estado Actual ===\n");
    printf("Escenario: %s\n", esc->nombre);
    printf("Descripción: %s\n", esc->descripcion);
    printf("Tiempo restante: %d\n\n", jugador->tiempo_restante);

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

    printf("Direcciones disponibles:\n");
    char *direcciones[] = {"Arriba", "Abajo", "Izquierda", "Derecha"};
    for (int i = 0; i < 4; i++) {
        if (esc->conexiones[i] != NULL)
            printf("%s\n", direcciones[i]);
    }
}

int avanzar(Jugador *jugador) {
    Escenario *esc = jugador->posicion_actual;
    printf("Seleccione dirección para avanzar:\n");
    char *direcciones[] = {"Arriba", "Abajo", "Izquierda", "Derecha"};
    int opciones_validas[4] = {0, 0, 0, 0};
    int contador_opc = 0;
    int i = 0;
    while (i < 4) {
        if (esc->conexiones[i] != NULL) {
            contador_opc++;
            printf("  %d) %s\n", contador_opc, direcciones[i]);
            opciones_validas[contador_opc - 1] = i;
        }
        i++;
    }
    if (contador_opc == 0) {
        printf("No hay direcciones disponibles para avanzar.\n");
        return 0; // No avanza
    }

    int opcion;
    scanf("%d", &opcion);
    if (opcion < 1 || opcion > contador_opc) {
        printf("Opción inválida.\n");
        return 0;
    }

    int dir = opciones_validas[opcion - 1];
    Escenario *nuevo_esc = esc->conexiones[dir];
    jugador->posicion_actual = nuevo_esc;

    printf("DEBUG: En %s, conexiones disponibles:\n", nuevo_esc->nombre);
    for (int i = 0; i < 4; i++) {
        if (nuevo_esc->conexiones[i] != NULL) {
            printf("  %d) %s -> %s\n", i + 1, direcciones[i], nuevo_esc->conexiones[i]->nombre);
        } else {
            printf("  %d) %s -> NULL\n", i + 1, direcciones[i]);
        }
    }   

    int tiempo_gastado = (int)((jugador->peso_actual + 1) / 10.0);
    jugador->tiempo_restante -= tiempo_gastado;

    printf("Avanzaste a %s. Tiempo usado: %d\n", nuevo_esc->nombre, tiempo_gastado);

    mostrar_estado(jugador);

    if (nuevo_esc->esFinal) {
        printf("¡Has llegado al escenario final!\n");
        printf("Inventario final y puntaje:\n");
        if (jugador->inventario == NULL || list_size(jugador->inventario) == 0) {
            printf("  (Inventario vacío)\n");
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
            printf("Puntaje final: %d\n", total_valor);
        }
        return 1; // fin de juego
    }

    if (jugador->tiempo_restante <= 0) {
        printf("Se agotó el tiempo. Has perdido.\n");
        return 1; // fin de juego
    }
    return 0; // seguir jugando
}

void list_popIndex(List *L, int index) {
    if (L == NULL || index < 0 || index >= list_size(L)) return;

    void *data = list_first(L);
    int i = 0;

    while (i < index) {
        data = list_next(L);
        i++;
    }

    list_popCurrent(L);
    free(data);
}

void recoger_items(Jugador *jugador) {
    Escenario *esc_aux = jugador->posicion_actual;

    if (esc_aux->items == NULL || list_size(esc_aux->items) == 0) {
        printf("No hay ítems para recoger en este escenario.\n");
        return;
    }

    printf("Ítems disponibles para recoger:\n");
    int i = 1;
    Item *item_aux = list_first(esc_aux->items);
    while (item_aux != NULL) {
        printf("  %d) %s (Peso: %d, Valor: %d)\n", i, item_aux->nombre, item_aux->peso, item_aux->valor);
        item_aux = list_next(esc_aux->items);
        i++;
    }

    printf("Ingrese el número del ítem que desea recoger (0 para cancelar): ");
    int opcion;
    scanf("%d", &opcion);

    if (opcion <= 0 || opcion > list_size(esc_aux->items)) {
        printf("Operación cancelada o opción inválida.\n");
        return;
    }

    // Mover el puntero al ítem deseado
    item_aux = list_first(esc_aux->items);
    for (int j = 1; j < opcion; j++) {
        item_aux = list_next(esc_aux->items);
    }

    // Clonar el ítem y agregarlo al inventario
    Item *nuevo = (Item *)malloc(sizeof(Item));
    strcpy(nuevo->nombre, item_aux->nombre);
    nuevo->peso = item_aux->peso;
    nuevo->valor = item_aux->valor;

    list_pushBack(jugador->inventario, nuevo);
    jugador->peso_actual += nuevo->peso;
    jugador->puntaje += nuevo->valor;

    // Eliminar el ítem del escenario
    list_popIndex(esc_aux->items, opcion - 1);
    free(item_aux);

    printf("Recogiste el ítem '%s'.\n", nuevo->nombre);
}

void descartar_items(Jugador *jugador) {
    if (jugador->inventario == NULL || list_size(jugador->inventario) == 0) {
        printf("No tienes ítems en tu inventario.\n");
        return;
    }

    printf("Ítems en tu inventario:\n");
    int i = 1;
    Item *item_aux = list_first(jugador->inventario);
    while (item_aux != NULL) {
        printf("  %d) %s (Peso: %d, Valor: %d)\n", i, item_aux->nombre, item_aux->peso, item_aux->valor);
        item_aux = list_next(jugador->inventario);
        i++;
    }

    printf("Ingrese el número del ítem que desea descartar (0 para cancelar): ");
    int opcion;
    scanf("%d", &opcion);

    if (opcion <= 0 || opcion > list_size(jugador->inventario)) {
        printf("Operación cancelada o opción inválida.\n");
        return;
    }
    
    item_aux = list_first(jugador->inventario);
    for (int j = 1; j < opcion; j++) {
        item_aux = list_next(jugador->inventario);
    }

    jugador->peso_actual -= item_aux->peso;
    jugador->puntaje -= item_aux->valor;

    printf("Descartaste el ítem '%s'.\n", item_aux->nombre);
    list_popIndex(jugador->inventario, opcion - 1);
    free(item_aux);
}

void iniciar_juego(List *lista_escenarios, int num_jugadores) {
    Jugador jugador1, jugador2;
    jugador1.inventario = list_create();
    jugador1.posicion_actual = list_first(lista_escenarios);
    jugador1.peso_actual = 0;
    jugador1.puntaje = 0;
    jugador1.tiempo_restante = 10;

    if (num_jugadores == 2) {
        jugador2.inventario = list_create();
        jugador2.posicion_actual = list_first(lista_escenarios);
        jugador2.peso_actual = 0;
        jugador2.puntaje = 0;
        jugador2.tiempo_restante = 10;
    }

    int fin = 0;
    int turno = 1;

    do {
        Jugador *jugador_actual;
        if (turno == 1) {
            jugador_actual = &jugador1;
        } else {
            jugador_actual = &jugador2;
        }

        printf("\n--- Turno del Jugador %d ---\n", turno);
        mostrar_estado(jugador_actual);

        printf("\n");
        printf("Opciones:\n");
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
                recoger_items(jugador_actual);
                break;
            case '2':
                descartar_items(jugador_actual);
                break;
            case '3':
                fin = avanzar(jugador_actual);
                break;
            case '4':
                // Reiniciar ambos jugadores si hay 2 jugadores
                list_clean(jugador1.inventario);
                jugador1.posicion_actual = list_first(lista_escenarios);
                jugador1.peso_actual = 0;
                jugador1.puntaje = 0;
                jugador1.tiempo_restante = 10;

                if (num_jugadores == 2) {
                    list_clean(jugador2.inventario);
                    jugador2.posicion_actual = list_first(lista_escenarios);
                    jugador2.peso_actual = 0;
                    jugador2.puntaje = 0;
                    jugador2.tiempo_restante = 10;
                }
                printf("Partida reiniciada.\n");
                printf("Volviendo al Menú Principal.\n");
            case '5':
                fin = 1;
                break;
            default:
                printf("Opción inválida.\n");
        }

        if (!fin && num_jugadores == 2) {
            if (turno == 1) {
                turno = 2;
            } else {
                turno = 1;
            }
        }

        presioneTeclaParaContinuar();
        limpiarPantalla();

    } while (!fin);

    // Liberar memoria de inventarios
    while (list_size(jugador1.inventario) > 0) {
        Item *it = list_first(jugador1.inventario);
        list_popFront(jugador1.inventario);
        free(it);
    }
    list_clean(jugador1.inventario);

    if (num_jugadores == 2) {
        while (list_size(jugador2.inventario) > 0) {
            Item *it = list_first(jugador2.inventario);
            list_popFront(jugador2.inventario);
            free(it);
        }
        list_clean(jugador2.inventario);
    }
}

int main() {
    List *lista_escenarios = list_create();
    char opcion;
    int escenarios_cargados = 0;
    do {
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
                leer_escenarios(lista_escenarios);
                Escenario *aux = list_first(lista_escenarios);
                if (aux != NULL) {
                    escenarios_cargados = 1;  // marcar que ya cargamos
                    printf("Laberinto cargado Exitosamente.\n");
                } else {
                    printf("No se cargo el Laberinto.\n\n");
                }
            } else {
                printf("El Laberinto ya fue cargado previamente.\n");
            }
            break;
        case '2':
            if (escenarios_cargados == 1) iniciar_juego(lista_escenarios, 1);
            else {
                printf("No se cargo el Laberinto.\n");
            }
            break;
        case '3':
            if (escenarios_cargados == 1) iniciar_juego(lista_escenarios, 2);
            else {
                printf("No se cargo el Laberinto.\n");
            }
            break;
        }
        presioneTeclaParaContinuar();
        limpiarPantalla();

    } while (opcion != '4');

    return 0;
}