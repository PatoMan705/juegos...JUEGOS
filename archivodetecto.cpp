/*
SDL BASICO

Definir limites de movimiento delos jugadores  CONSEGUIDO 18/06/2025 21:04
hacer movimeinto de la pelota                   CONSEGUIDO 21/06/2025 01:09
hacer las colisiones de la pelota con los bordes CONSEGUIDO 21/06/2025 01:45
hacer las colisiones de la pelota con los jugadores     CONSEGUIDO PARCIALMENTE, despues de todos los cuba libres nose como es que anda
CONSEGUIDO 22/06/2025 tras razonar un poco mejor los codigos de mi yo borracho, los hice andar, que cosa copada
extra, hacer posible jugagar en multiplayer
agregar colores bonitos.

SDL  TTF

hacer diferentes pantallas [Manjarse con WASD/Flechitas y Enter y/o mouse y click]
Inicio (Jugar, opciones, salir)
Juego ( puntaje de cada uno, Tiempo restante)
Pantalla de Pausa (Reanudar, Salir)

SDL MIXER
añadir sonidos a colisiones
añadir sonidos al marcar un punto
añadir sonidos al desplazarse por los menus

PONG MEJORAS ULTRASECRETAS SECRETISIMAS

SDL MIXER / TTF / IMAGE
cargar el fondo osiloscopio
iniciar el pong en modo osiloscopio
Añadir a lo anterior soniditos de osiloscopio

*/


#include <SDL.h> //SDL basico
#include <SDL_mixer.h> //audio con sld
#include <SDL_image.h> //imagenes con esedeele
#include <SDL_ttf.h> //manejar fuentes de texto en pantalla

#include <fstream> // manejo de archivos

#include <iostream>
#include <Windows.h>
#include <time.h>
#include <string>

using namespace std;
/*------------------------------------------------------------------------------------------------------------------------------------------------------
* Debido a que mis habilidades de programacion no son las mejores, debido a pequenieces y peculiaridades de los calculos involucrados, no puedo modificar
* a voluntad la velocidad del la pelota o la pantalla sin romper algo de por medio, vamos a guardar algunos modos que sabemos que funcionan
* una ves terminado el testing y llamemosles dia completo >:(>
------------------------------------------------------------------------------------------------------------------------------------------------------*/
const int WIN_WIDTH = 1200;
const int WIN_HEIGHT = 600;
const int FLAP_HEIGHT = (WIN_WIDTH / 10);
const int FLAP_WIDTH = (WIN_HEIGHT / 20);
const int VEL_FLAP = 15;
const int VEL_PELOTA = 10;

const int A = 10;

void guardarPuntaje(string nombreArchivo, string nombreJugador, int puntajeJugador) {

    fstream archivo;
    archivo.open(nombreArchivo + ".csv", ios::app); // ios::in escribe al inicio y sobre escribe; ios::app escribe al final sin jorobar otros datos.

    if (archivo.is_open()) {
        archivo << nombreJugador << ";" << puntajeJugador << endl;
    }
    else cerr << "Error al abrir/crear el archivo " << endl;

    archivo.close(); //siempre que se habre un archivo, se cierra al final.
}

void gameloop() {


}

int main(int argc, char* argv[]) {

    srand(time(NULL)); //inicializamos la semilla

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "Error al iniciar SDL: " << SDL_GetError() << endl;
        return 1;
    }

    //genero un struct de nombre window usando la funcion de SDL_CreateWindow
    //le paso el nombre de la ventana
    //luego el eje x, luego el y,
    //luego ancho, luego alto,
    //luego una FLAG propia de la funcion la 4 es la shown, la 3 sirve para pantalla completa

    SDL_Window* window = SDL_CreateWindow(
        "Pong De Recreo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH, WIN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (window == nullptr) {
        cerr << "Error al crear la ventana: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == nullptr) {
        cerr << "Error al crear el renderer: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        cerr << "Error al iniciar SDL_image: " << IMG_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /*---------------------------------------------------------------------------------------------------------
    Para definir las posiciones inciales de los flaps utilice las propias medidas de los mimsmos como separadores
    de esta forma en un futuro si quiero cambiar el ancho y largo de la pantalla puedo hacerlo sin ningun problema
    y en sistesis no se rompe nada :) nota menta 22/06/2025. ahora que vimos structs, la proxima vez lo metes todos
    estos calculos raros en el struct y les das nombres adecuados, pero para la proxima, ahora no nos da el tiempo.
    22/06/2025 15:27 alfinal se cambio todo por structs xd
    ----------------------------------------------------------------------------------------------------------------*/

    struct Flaper {
        SDL_Rect rect;
        int vel = VEL_FLAP;
        int velCPU = VEL_FLAP * 0.5;
        int puntaje = 0;
        bool cpu = 0;
        bool ganar = 0;
    };

    Flaper jugadorIzq;

    jugadorIzq.rect = { FLAP_WIDTH, 0, FLAP_WIDTH, FLAP_HEIGHT };
    jugadorIzq.cpu = 1;

    Flaper jugadorDer;

    jugadorDer.rect = { WIN_WIDTH - (FLAP_WIDTH * 2), 0, FLAP_WIDTH, FLAP_HEIGHT };
    jugadorDer.cpu = 1;

    struct bola {               //creo un sruct para meteri todo lo importante de las bolas en un mismo lugar, quizas añada un multiball
        float velX = VEL_PELOTA;
        float velY = VEL_PELOTA;
        SDL_Rect rect = { (WIN_WIDTH / 2) , WIN_HEIGHT / 2, 10, 10 };
    };

    bola pelotita;

    SDL_Event event;

    //defino diferentes estados utiles
    struct estadosDeJuego {
        bool sound = 1;
        bool running = 1;
        bool pause = 0;
        int tiempo = 120;
    };

    estadosDeJuego game;

    struct pantallas {

        SDL_Rect titulo{ WIN_WIDTH / 3,0,WIN_WIDTH * 2 / 3,WIN_HEIGHT / 3 };
        SDL_Rect inicio{ titulo.x, 200, 400, 400};
        SDL_Rect opciones {inicio.x, inicio.y+FLAP_WIDTH, 200, 80};
        SDL_Rect salir{ opciones.x, opciones.y + FLAP_WIDTH, 200, 80 };

    };

    pantallas menu;

    while (game.running) {

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &menu.titulo);



        //mostrar en pantalla
        SDL_RenderPresent(renderer);
    }

    //GAMELOOP HERE
    while (game.running) {
        //me dijo un pajarito que esto anda pero ni idea
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game.running = false;
            }
        }

        const Uint8* keystates = SDL_GetKeyboardState(NULL);                 //esto de aca te permite leer el estado del teclado, NECESARIO para manejar multiples inputs (multiplayer)

        //movimiento para flapIzquierdo (WS) delimitados por la pantalla
        //movimiento para Jugador Derecho
        if (jugadorIzq.cpu) { //si es una maquina le doy esta ia simple y se termino =)

            if (jugadorIzq.rect.y >= 0 && (pelotita.rect.y <= (jugadorIzq.rect.y + (FLAP_HEIGHT / 2))))
            {
                jugadorIzq.rect.y -= jugadorIzq.velCPU;
            }
            if (jugadorIzq.rect.y <= (WIN_HEIGHT - FLAP_HEIGHT) && (pelotita.rect.y >= (jugadorIzq.rect.y + (FLAP_HEIGHT / 2))))
            {
                jugadorIzq.rect.y += jugadorIzq.velCPU;
            }
        }
        else { //si no es cpu le permito escanear el teclado
            if (keystates[SDL_SCANCODE_W] && jugadorIzq.rect.y >= 0) jugadorIzq.rect.y -= jugadorIzq.vel;
            if (keystates[SDL_SCANCODE_S] && jugadorIzq.rect.y <= (WIN_HEIGHT - FLAP_HEIGHT)) jugadorIzq.rect.y += jugadorIzq.vel;
        }

        //movimiento para Jugador Derecho
        if (jugadorDer.cpu) { //si es una maquina le doy esta ia simple y se termino =)

            if (jugadorDer.rect.y >= 0 && (pelotita.rect.y <= (jugadorDer.rect.y + (FLAP_HEIGHT / 2))))
            {
                jugadorDer.rect.y -= jugadorDer.velCPU;
            }
            if (jugadorDer.rect.y <= (WIN_HEIGHT - FLAP_HEIGHT) && (pelotita.rect.y >= (jugadorDer.rect.y + (FLAP_HEIGHT / 2))))
            {
                jugadorDer.rect.y += jugadorDer.velCPU;
            }

        }
        else { //si no es cpu le permito escanear el teclado
            if (keystates[SDL_SCANCODE_UP] && jugadorDer.rect.y >= 0) jugadorDer.rect.y -= jugadorDer.vel;
            if (keystates[SDL_SCANCODE_DOWN] && jugadorDer.rect.y <= (WIN_HEIGHT - FLAP_HEIGHT)) jugadorDer.rect.y += jugadorDer.vel;
        }


        //teclas de accesibilidad
        if (keystates[SDL_SCANCODE_Q]) game.running = false;
        if (keystates[SDL_SCANCODE_M]) game.sound = false;



        //fondo
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        //dibujo la pelotita de blanco
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        SDL_RenderFillRect(renderer, &pelotita.rect);

        //manjeamos colisiones en borde superior e inferior
        if (pelotita.rect.y <= 0 || pelotita.rect.y >= WIN_HEIGHT) {
            pelotita.velY = -pelotita.velY;
        }

        //chequeamos si el jugador 2 anoto un punto y reseteamos la pelota
        if (pelotita.rect.x >= WIN_WIDTH) {
            jugadorDer.puntaje++;
            pelotita.rect.y = WIN_HEIGHT / 2;
            pelotita.rect.x = WIN_WIDTH / 2;
            game.pause = 1;
            pelotita.velX = -pelotita.velX;
        }

        //chequeamos si el jugador 2 anoto un punto y reseteamos la pelota
        if (pelotita.rect.x <= 0) {
            jugadorIzq.puntaje++;
            pelotita.rect.y = WIN_HEIGHT / 2;
            pelotita.rect.x = WIN_WIDTH / 2;
            game.pause = 1;
            pelotita.velX = -pelotita.velX;
        }

        //chequeamos colisiones con la pelota en la cara interna del jugador 2
        if (((FLAP_WIDTH * 2) == pelotita.rect.x) && (jugadorIzq.rect.y <= pelotita.rect.y) && (pelotita.rect.y <= (jugadorIzq.rect.y + FLAP_HEIGHT))) {
            pelotita.velX = -pelotita.velX;
        }

        //chequeamos colisiones con la pelota en la cara interna del jugador 1
        if (((WIN_WIDTH - (FLAP_WIDTH * 2)) == pelotita.rect.x) && (jugadorIzq.rect.y <= pelotita.rect.y) && (pelotita.rect.y <= (jugadorIzq.rect.y + FLAP_HEIGHT))) {
            pelotita.velX = -pelotita.velX;
        }

        //manejamos el movimiento de la pelota
        pelotita.rect.x += pelotita.velX;
        pelotita.rect.y += pelotita.velY;

        //dibujar el flapIzquierdo
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &jugadorIzq.rect);


        //dibujar el flapDerecho
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &jugadorDer.rect);


        //mostrar en pantalla
        SDL_RenderPresent(renderer);

        if (game.pause) {
            SDL_Delay(1250);
            game.pause = 0;
        }
        SDL_Delay(16); //demora de dibjuado entre bucles
    }

    //limpiamos la memoria
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
