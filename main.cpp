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
const int WINX_MID = WIN_WIDTH / 2;
const int WINY_MID = WIN_HEIGHT / 2;
const int FLAP_HEIGHT = (WIN_WIDTH / 10);
const int FLAP_WIDTH = (WIN_HEIGHT / 20);
const int VEL_FLAP = 15;
const int VEL_PELOTA = 10;

const int A = 10;
//esto de aca te permite leer el estado del teclado, NECESARIO para manejar multiples inputs (multiplayer)
const Uint8* keystates = SDL_GetKeyboardState(NULL);

struct cuadroTexto {

    bool select = 0;
    int fontSize;
    SDL_Rect rect;
    string text;
    SDL_Color color = { 255, 255, 255 };
    SDL_Color colorB = { 200, 200, 200 };
    SDL_Texture* texture = nullptr;
};

void guardarPuntaje(string nombreArchivo, string Jugador1, int puntajeJugador1, string Jugador2, int puntajeJugador2) {

    fstream archivo;
    archivo.open(nombreArchivo + ".csv", ios::app); // ios::in escribe al inicio y sobre escribe; ios::app escribe al final sin jorobar otros datos.

    //logica para definir quien gano

    if (archivo.is_open()) {
        archivo << Jugador1 << ";" << puntajeJugador1 << endl;
    }
    else cerr << "Error al abrir/crear el archivo " << endl;

    archivo.close(); //siempre que se habre un archivo, se cierra al final.
}

SDL_Texture* updateTexture(SDL_Renderer* renderer, TTF_Font* font, cuadroTexto& cuadro) {

    // Liberar textura anterior si existe
    if (cuadro.texture) {
        SDL_DestroyTexture(cuadro.texture);
        cuadro.texture = nullptr;
    }

    // Crear superficie
    SDL_Surface* surface = TTF_RenderText_Solid(font, cuadro.text.c_str(), cuadro.color);
    if (!surface) {
        cerr << "Error al crear superficie para '" << cuadro.text << "': " << TTF_GetError() << endl;
        return nullptr;
    }

    // Crear textura
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        cerr << "Error al crear textura para '" << cuadro.text << "': " << SDL_GetError() << endl;
        SDL_FreeSurface(surface);
        return nullptr;
    }

    SDL_FreeSurface(surface);
    cuadro.texture = texture;
    return texture;
}

void meterTexto(SDL_Renderer* renderer, TTF_Font* font, cuadroTexto& cuadro) {
    // Asegurarse de que la textura esté actualizada
    if (!cuadro.texture) {
        updateTexture(renderer, font, cuadro);
    }
    // Renderizar textura
    if (cuadro.texture) {
        SDL_RenderCopy(renderer, cuadro.texture, NULL, &cuadro.rect);
    }
}

void gamemenu(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font,
    cuadroTexto& titulo, cuadroTexto& inicio, cuadroTexto& opciones, cuadroTexto& salir, cuadroTexto& yo) {
    bool ismenuon = true;
    SDL_Event event;
    int selectedOption = 0; // 0: inicio, 1: opciones, 2: salir

    while (ismenuon) {
        // Manejo de eventos
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                ismenuon = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    selectedOption = (selectedOption - 1 + 3) % 3; // Ciclo hacia arriba
                    break;
                case SDLK_DOWN:
                    selectedOption = (selectedOption + 1) % 3; // Ciclo hacia abajo
                    break;
                case SDLK_q:
                    ismenuon = false;
                    break;
                }
            }
        }

        // Actualizar selección
        inicio.select = (selectedOption == 0);
        opciones.select = (selectedOption == 1);
        salir.select = (selectedOption == 2);

        // Actualizar colores según selección
        inicio.color = inicio.select ? SDL_Color{ 0, 255, 0, 255 } : SDL_Color{ 255, 255, 255, 255 };
        opciones.color = opciones.select ? SDL_Color{ 0, 255, 0, 255 } : SDL_Color{ 255, 255, 255, 255 };
        salir.color = salir.select ? SDL_Color{ 0, 255, 0, 255 } : SDL_Color{ 255, 255, 255, 255 };

        // Actualizar texturas
       
        updateTexture(renderer, font, inicio);
        updateTexture(renderer, font, opciones);
        updateTexture(renderer, font, salir);
        // No actualizamos titulo porque no cambia

        // Limpieza de pantalla
        SDL_SetRenderDrawColor(renderer, 7, 71, 1, 255); // Verde oscuro
        SDL_RenderClear(renderer);

        // Dibujar textos con meterTexto
        meterTexto(renderer, font, yo);
        meterTexto(renderer, font, titulo);
        meterTexto(renderer, font, inicio);
        meterTexto(renderer, font, opciones);
        meterTexto(renderer, font, salir);

        // Mostrar en pantalla
        SDL_RenderPresent(renderer);

        // Controlar la velocidad
        SDL_Delay(16); // ~60 FPS
    }
}


void gameloop(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
        /*---------------------------------------------------------------------------------------------------------
       Para definir las posiciones inciales de los flaps utilice las propias medidas de los mimsmos como separadores
       de esta forma en un futuro si quiero cambiar el ancho y largo de la pantalla puedo hacerlo sin ningun problema
       y en sistesis no se rompe nada :) nota menta 22/06/2025. ahora que vimos structs, la proxima vez lo metes todos
       estos calculos raros en el struct y les das nombres adecuados, pero para la proxima, ahora no nos da el tiempo.
       22/06/2025 15:27 alfinal se cambio todo por structs xd
       ------------------------------------------------------------------------------------------------------------*/
    struct Flaper {
        SDL_Rect rect;
        int vel = VEL_FLAP;
        int velCPU = VEL_FLAP * 0.5;
        int puntaje = 0;
        bool cpu = 0;
        bool ganar = 0;
        string nombre;
    };
    struct bola {               //creo un sruct para meteri todo lo importante de las bolas en un mismo lugar, quizas añada un multiball
        float velX = VEL_PELOTA;
        float velY = VEL_PELOTA;
        SDL_Rect rect = { (WIN_WIDTH / 2) , WIN_HEIGHT / 2, 10, 10 };
    };

    bola pelotita;
    Flaper jugadorIzq;
    jugadorIzq.nombre = "Scorch";
    jugadorIzq.rect = { FLAP_WIDTH, 0, FLAP_WIDTH, FLAP_HEIGHT };
    jugadorIzq.cpu = 1;

    Flaper jugadorDer;
    jugadorIzq.nombre = "Legion";
    jugadorDer.rect = { WIN_WIDTH - (FLAP_WIDTH * 2), 0, FLAP_WIDTH, FLAP_HEIGHT };
    jugadorDer.cpu = 1;

    SDL_Event event;

    //defino diferentes estados utiles
    struct estadosDeJuego {
        bool sound = 1;
        bool running = 1;
        bool pause = 0;
        int tiempo = 120;
    };

    estadosDeJuego game;

    //GAMELOOP HERE BOY
    while (game.running) {
        //me dijo un pajarito que esto anda pero ni idea
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game.running = false;
            }
        }

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
        SDL_SetRenderDrawColor(renderer, 7, 71, 1, 255);
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
            SDL_Delay(500);
            game.pause = 0;
        }
        SDL_Delay(16); //demora de dibjuado entre bucles
    }

    //guardarPuntaje("Puntajes","") guardar ahciovos
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

    SDL_Window* window = SDL_CreateWindow ("Pong De Recreo",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);

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
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("assets\\Chalkboard.ttf", 24);
    if (font == NULL) {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, "hello there, general kenobi", {255, 255, 255 });
    if (!surface) {
        cerr << "Error al crear la superficie de texto: " << TTF_GetError() << endl;
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        cerr << "Error al crear la textura de texto: " << SDL_GetError() << endl;
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    
    cuadroTexto titulo;
    titulo.rect = { WIN_WIDTH / 5, 0, WIN_WIDTH * 3 / 5, WIN_HEIGHT / 4 };
    titulo.text = "PONG DE RECREO";

    cuadroTexto inicio;
    inicio.rect = { WINX_MID - FLAP_HEIGHT, WIN_WIDTH / 5, WIN_WIDTH / 5, WIN_HEIGHT / 10 };
    inicio.text = "INICIO";

    cuadroTexto opciones;
    opciones.rect = { inicio.rect.x, inicio.rect.y + FLAP_WIDTH + WIN_HEIGHT / 10, WIN_WIDTH / 5, WIN_HEIGHT / 10 };
    opciones.text = "OPCIONES";

    cuadroTexto salir;
    salir.rect = { inicio.rect.x, opciones.rect.y + FLAP_WIDTH + WIN_HEIGHT / 10, WIN_WIDTH / 5, WIN_HEIGHT / 10 };
    salir.text = "SALIR";

    cuadroTexto yo;
    yo.rect = { 0, WIN_HEIGHT - WIN_HEIGHT / 10, WIN_WIDTH / 4, 50 };
    yo.text = "MURGIA AGUSTIN UADE L:1220705";

   // gamemenu(window, renderer, font, titulo, inicio, opciones, salir, yo);

    gameloop(window, renderer, font);



    //Liberar texturas
    SDL_DestroyTexture(titulo.texture);
    SDL_DestroyTexture(inicio.texture);
    SDL_DestroyTexture(opciones.texture);
    SDL_DestroyTexture(salir.texture);
    //limpiamos la memoria
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
