#include <iostream>
#include <vector>
#include <unistd.h>    // Para usleep()
#include <termios.h>   // Para configuración de terminal
#include <fcntl.h>     // Para manipulación de descriptores de archivo
#include <cstdlib>     // Para system("clear")

using namespace std;

// Constantes del juego
const char PARED = '#';
const char CAMINO = ' ';
const char PACMAN = 'P';
const char PUNTO = '.';
const int FILAS = 13;
const int COLUMNAS = 27;

// Laberinto inicial basado en el ejemplo del PDF
vector<string> laberinto = {
    "###########################",
    "#P...........#...........#",
    "#.#####.#####.#.#####.###.#",
    "#.#   #.#   #.#.#   #.# #.#",
    "#.#   #.#   #.#.#   #.# #.#",
    "#.#####.#####.#.#####.###.#",
    "#.........................#",
    "###.#####.#######.#####.###",
    "#   #.#   #     #.#   #   #",
    "#   #.#   #     #.#   #   #",
    "#.###.###.#.#####.#.###.###",
    "#...........#.............#",
    "###########################"
};

// Posiciones de Pac-Man
int pacmanX = 1, pacmanY = 1;
int puntos = 0;

// Configuración del terminal para entrada no bloqueante
void configurarTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON; // Deshabilitar entrada canónica (línea por línea)
    t.c_lflag &= ~ECHO;   // Deshabilitar eco de teclas
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void restaurarTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON;  // Habilitar entrada canónica
    t.c_lflag |= ECHO;    // Habilitar eco de teclas
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Verifica si hay entrada disponible en el teclado
bool teclaPresionada() {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

// Captura una tecla presionada
char capturarTecla() {
    char tecla;
    read(STDIN_FILENO, &tecla, 1);
    return tecla;
}

// Imprime el laberinto y el puntaje actual
void imprimirLaberinto() {
    system("clear");
    for (const auto &fila : laberinto) {
        cout << fila << endl;
    }
    cout << "Puntaje: " << puntos << endl;
    cout << "Controles: z (abajo), s (arriba), d (derecha), a (izquierda), q (salir)" << endl;
}

// Mueve a Pac-Man en la dirección especificada
void moverPacman(char direccion) {
    int nuevoX = pacmanX, nuevoY = pacmanY;

    if (direccion == 'z') nuevoX++;     // Abajo
    else if (direccion == 's') nuevoX--; // Arriba
    else if (direccion == 'd') nuevoY++; // Derecha
    else if (direccion == 'a') nuevoY--; // Izquierda

    // Teletransportación entre los bordes del laberinto
    if (nuevoY < 0) nuevoY = COLUMNAS - 1;
    if (nuevoY >= COLUMNAS) nuevoY = 0;

    // Verifica colisiones y movimiento
    if (laberinto[nuevoX][nuevoY] != PARED) {
        if (laberinto[nuevoX][nuevoY] == PUNTO) {
            puntos += 10; // Incrementa puntaje por comer un punto
        }
        laberinto[pacmanX][pacmanY] = CAMINO; // Limpia posición anterior
        pacmanX = nuevoX;
        pacmanY = nuevoY;
        laberinto[pacmanX][pacmanY] = PACMAN; // Actualiza nueva posición
    }
}

// Verifica si todos los puntos han sido recolectados
bool verificarVictoria() {
    for (const auto &fila : laberinto) {
        if (fila.find(PUNTO) != string::npos) {
            return false;
        }
    }
    return true;
}

int main() {
    configurarTerminal();
    atexit(restaurarTerminal);

    char tecla;
    bool jugando = true;

    imprimirLaberinto();

    while (jugando) {
        if (teclaPresionada()) {
            tecla = capturarTecla();
            tecla = tolower(tecla); // Acepta teclas en minúscula y mayúscula
            if (tecla == 'q') {
                jugando = false; // Salir del juego
            } else {
                moverPacman(tecla);
                imprimirLaberinto();
                if (verificarVictoria()) {
                    cout << "¡Felicidades! Has recolectado todos los puntos. Puntaje final: " << puntos << endl;
                    jugando = false;
                }
            }
        }
        usleep(100000); // Pausa breve para controlar la velocidad
    }

    cout << "Gracias por jugar." << endl;
    return 0;
}
