# Graphquest

## Descripción

Este programa permite jugar un juego llamado GraphQuest, en el cual hay que recorrer un laberinto almacenado en un grafo donde cada nodo es un escenario distinto, donde el jugador debe decidir hacia donde moverse y qu[e objetos va a tomar o dejar. Cada elemento tiene un valor y un peso, y el jugador tiene un tiempo limitado para completar el laberinto, el cual cambia dependiendo del peso de los objetos que lleve.

## Cómo compilar y ejecutar

Este sistema ha sido desarrollado en lenguaje C y puede ejecutarse fácilmente utilizando **Visual Studio Code** junto con una extensión para C/C++, como **C/C++ Extension Pack** de Microsoft. Para comenzar a trabajar con el sistema en tu equipo local, sigue estos pasos:

### Requisitos previos:

- Tener instalado [Visual Studio Code](https://code.visualstudio.com/).
- Instalar la extensión **C/C++** (Microsoft).
- Tener instalado un compilador de C (como **gcc**). Si estás en Windows, se recomienda instalar [MinGW](https://www.mingw-w64.org/) o utilizar el entorno [WSL](https://learn.microsoft.com/en-us/windows/wsl/).

### Pasos para compilar y ejecutar:

1. **Descarga y descomprime el** archivo `.zip` en una carpeta de tu elección.
2. **Abre el proyecto en Visual Studio Code**
    - Inicia Visual Studio Code.
    - Selecciona `Archivo > Abrir carpeta...` y elige la carpeta donde descomprimiste el proyecto.
3. **Compila el código**
    - Abre el archivo principal (`tarea3.c)
    - Abre la terminal integrada (`Terminal > Nueva terminal`).
    - En la terminal, compila el programa con el siguiente comando (ajusta el nombre si el archivo principal tiene otro nombre):
        
        ```powershell
        gcc -o tarea tarea3.c tdas/list.c tdas/extra.c -Wall -Wextra 
        ```
        
4. **Ejecuta el programa**
    - Una vez compilado, puedes ejecutar la aplicación con:
        
        ```
        ./tarea.exe
        ```
        

## Funcionalidades

### Funcionando correctamente:

- Cargar laberinto desde el archivo tipo csv.
- Iniciar partida de 1 o 2 jugadores.
- Describir el escenario y estado del jugador en cada jugada.
- Recoger o descartar items.
- Avanzar en las direcciones disponibles.
- Reiniciar partida.

### Problemas conocidos:

-

### A mejorar:

- Implementar función para que el programa reconozca artistas/géneros sin que se escriban exactamente igual.
- Implementar función para eliminar playlist y para editar estas, permitiendo borrar canciones y cambiar nombre.
- Implementar función para buscar canciones por su nombre.








## Ejemplo de uso

**1: Cargar laberinto**

Se comienza cargando el laberinto desde el archivo .csv

```
Opción seleccionada: 1) Cargar laberinto
Laberinto cargado Exitosamente.
```

Si se intenta cargar de nuevo sin finalizar la partida, se indica y no se carga otra vez.

**2: Iniciar Partida Solo**

Se inicia partida de un solo jugador, solo si el laberinto ya fue cargado
```
Opción seleccionada: 2) Iniciar Partida Solo 
Inventario del jugador:
Inventario vac├¡o
Direcciones disponibles:
Abajo
Opciones:
1) Recoger ├ìtem(s)
2) Descarta ├ìtem(s)
3) Avanzar en una Direcci├│n
4) Reiniciar Partida
5) Salir del Juego
```

Para cada turno, se muestra el inventario del jugador, las direcciones disponibles y las acciones que puede realizar el jugador.

**2.1: Recoger ├ìtem(s)**

```
No hay ├¡tems para recoger en este escenario.
```

Se muestran los objetos disponibles para recoger. Si no hay elementos se indica como en el ejemplo.

**2.2: Descartar ├ìtem(s)**

```
No tienes ├¡tems en tu inventario.
```

Se muestran los objetos en el inventario o se indica si este está vacío. Si hay items en este, se pueden descartar.

**2.3: Avanzar en una dirección**

```
Seleccione direcci├│n para avanzar:
  1) Abajo
```

Se muestran las direcciones disponibles para moverse. Al elegir una de las disponibles, el escenario cambia a la posición elegida.

**3: Iniciar partida Multijugador**

Tiene las mismas funciones que la partida solo y se comienza desde el mismo escenario, solo que se muestra todo lo mismo para el turno de cada jugador, por lo que hay que esperar a que el otro jugador termine su turno antes de cambiar de escenario.

**4: Salir**

