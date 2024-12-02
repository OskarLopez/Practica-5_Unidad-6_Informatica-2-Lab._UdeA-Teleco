#include <iostream>
#include <vector>
#include <unistd.h>    // Para usleep()
#include <termios.h>   // Para configuración de terminal
#include <cstdlib>     // Para system("clear")
#include <ctime>       // Para srand() y rand()
#include <sys/select.h> // Para select()
#include <sstream>     // Para evitar el parpadeo

using namespace std;

// Constantes del juego
const char PARED = '#';
const char CAMINO = ' ';
const char PACMAN = 'P';
const char FANTASMA = 'G';
const char PUNTO = '.';
const char FRUTA = 'F';

// Colores ANSI
const string COLOR_PACMAN = "\033[33m";  // Amarillo
const string COLOR_FANTASMA = "\033[31m"; // Rojo
const string COLOR_FRUTA = "\033[32m";    // Verde
const string COLOR_RESET = "\033[0m";    // Reset

// Laberintos por niveles
vector<vector<string>> niveles = {
    {
        "###########################",
        "#P...........#...........#",
        "#.#####.#####.#.#####.###.#",
        "#.#   #.#   #.#.#   #.# #.#",
        "#.#   #.#   #.#.#   #.# #.#",
        "#.#####.#####.#.#####.###.#",
        "#.........F...............#",
        "###.#####.#######.#####.###",
        "#   #.#   #     #.#   #   #",
        "#   #.#   #     #.#   #   #",
        "#.###.###.#.#####.#.###.###",
        "#...........#.............#",
        "###########################"
    },
    {
        "##############################",
        "#P.............#####........G#",
        "#.#####.#######.#...#.#######.#",
        "#.#   #.#     #.#.#.#.....#..#",
        "#.#   #.#     #.#.#.#######..#",
        "#.#####.#####.#.#.#.#....#...#",
        "#.........F....#...#....###..#",
        "#########.#####.#.#####.#####",
        "#.....#...#.....#...........#",
        "#.....#...#.###########.#####",
        "#.#####.###.......F.........#",
        "#....................#.......#",
        "##############################"
    }
};

// Posiciones de Pac-Man y enemigos
int pacmanX, pacmanY;
vector<pair<int, int>> fantasmas;
int puntos = 0;
int nivelActual = 0;

// Configuración de la terminal
void configurarTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON; // Deshabilitar entrada canónica
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

bool teclaPresionada() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = {0, 0};
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

char capturarTecla() {
    char tecla;
    read(STDIN_FILENO, &tecla, 1);
    return tecla;
}

void imprimirLaberinto(const vector<string> &laberinto) {
    ostringstream pantalla;
    for (size_t i = 0; i < laberinto.size(); ++i) {
        for (size_t j = 0; j < laberinto[i].size(); ++j) {
            if (laberinto[i][j] == PACMAN) {
                pantalla << COLOR_PACMAN << PACMAN << COLOR_RESET;
            } else if (laberinto[i][j] == FANTASMA) {
                pantalla << COLOR_FANTASMA << FANTASMA << COLOR_RESET;
            } else if (laberinto[i][j] == FRUTA) {
                pantalla << COLOR_FRUTA << FRUTA << COLOR_RESET;
            } else {
                pantalla << laberinto[i][j];
            }
        }
        pantalla << '\n';
    }
    pantalla << "Nivel: " << nivelActual + 1 << " | Puntos: " << puntos << '\n';
    pantalla << "Controles: z (abajo), s (arriba), d (derecha), a (izquierda), q (salir)" << endl;
    cout << "\033[H\033[J" << pantalla.str(); // Evita parpadeo
}

void inicializarNivel() {
    vector<string> &laberinto = niveles[nivelActual];
    fantasmas.clear();
    for (size_t i = 0; i < laberinto.size(); ++i) {
        for (size_t j = 0; j < laberinto[i].size(); ++j) {
            if (laberinto[i][j] == PACMAN) {
                pacmanX = i;
                pacmanY = j;
            } else if (laberinto[i][j] == FANTASMA) {
                fantasmas.emplace_back(i, j);
            }
        }
    }
}

void moverPacman(vector<string> &laberinto, char direccion) {
    int nuevoX = pacmanX, nuevoY = pacmanY;

    if (direccion == 'z') nuevoX++;
    else if (direccion == 's') nuevoX--;
    else if (direccion == 'd') nuevoY++;
    else if (direccion == 'a') nuevoY--;

    if (nuevoY < 0) nuevoY = laberinto[0].size() - 1;
    if (nuevoY >= static_cast<int>(laberinto[0].size())) nuevoY = 0;

    if (laberinto[nuevoX][nuevoY] != PARED) {
        if (laberinto[nuevoX][nuevoY] == PUNTO) puntos += 10;
        if (laberinto[nuevoX][nuevoY] == FRUTA) puntos += 50;
        laberinto[pacmanX][pacmanY] = CAMINO;
        pacmanX = nuevoX;
        pacmanY = nuevoY;
        laberinto[pacmanX][pacmanY] = PACMAN;
    }
}

void moverFantasmas(vector<string> &laberinto) {
    for (auto &fantasma : fantasmas) {
        int x = fantasma.first;
        int y = fantasma.second;

        int dx = (pacmanX > x) - (pacmanX < x); // Dirección hacia Pac-Man
        int dy = (pacmanY > y) - (pacmanY < y);

        // Movimiento aleatorio o hacia Pac-Man
        if (rand() % 2) dx = (rand() % 3) - 1;
        if (rand() % 2) dy = (rand() % 3) - 1;

        int nuevoX = x + dx;
        int nuevoY = y + dy;

        if (nuevoY < 0) nuevoY = laberinto[0].size() - 1;
        if (nuevoY >= static_cast<int>(laberinto[0].size())) nuevoY = 0;

        if (laberinto[nuevoX][nuevoY] == CAMINO || laberinto[nuevoX][nuevoY] == PACMAN) {
            laberinto[x][y] = CAMINO;
            fantasma.first = nuevoX;
            fantasma.second = nuevoY;
            laberinto[nuevoX][nuevoY] = FANTASMA;
        }
    }
}

bool verificarColision() {
    for (const auto &fantasma : fantasmas) {
        if (pacmanX == fantasma.first && pacmanY == fantasma.second) {
            return true;
        }
    }
    return false;
}

bool verificarVictoria(const vector<string> &laberinto) {
    for (const auto &fila : laberinto) {
        if (fila.find(PUNTO) != string::npos || fila.find(FRUTA) != string::npos) {
            return false;
        }
    }
    return true;
}

int main() {
    srand(time(NULL));
    configurarTerminal();
    atexit(restaurarTerminal);

    while (true) {
        vector<string> laberinto = niveles[nivelActual];
        inicializarNivel();

        while (true) {
            imprimirLaberinto(laberinto);
            if (verificarColision()) {
                cout << "¡Perdiste! Presiona cualquier tecla para salir." << endl;
                capturarTecla();
                exit(0);
            }

            if (verificarVictoria(laberinto)) {
                nivelActual = (nivelActual + 1) % niveles.size();
                break;
            }

            if (teclaPresionada()) {
                char tecla = capturarTecla();
                if (tecla == 'q') {
                    cout << "Saliendo del juego." << endl;
                    exit(0);
                }
                moverPacman(laberinto, tecla);
            }

            moverFantasmas(laberinto);
            usleep(100000); // Pausa breve
        }
    }

    return 0;
}
