#include <iostream>
#include <vector>
#include <unistd.h>    // Para usleep()
#include <termios.h>   // Para configuración de terminal
#include <cstdlib>     // Para system("clear")
#include <ctime>       // Para srand()
#include <sys/select.h> // Para select()
#include <sstream>     // Para evitar el parpadeo
#include <cmath>       // Para abs()
#include <algorithm>   // Para shuffle
#include <random>      // Para std::shuffle

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

// Variables globales
int pacmanX, pacmanY;
vector<pair<int, int>> fantasmas;
int puntos = 0;
size_t nivelActual = 0;

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
    }
};

// Configuración de la terminal ```cpp
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
    for (const auto &fila : laberinto) {
        for (char celda : fila) {
            if (celda == PACMAN) {
                pantalla << COLOR_PACMAN << PACMAN << COLOR_RESET;
            } else if (celda == FANTASMA) {
                pantalla << COLOR_FANTASMA << FANTASMA << COLOR_RESET;
            } else if (celda == FRUTA) {
                pantalla << COLOR_FRUTA << FRUTA << COLOR_RESET;
            } else {
                pantalla << celda;
            }
        }
        pantalla << '\n';
    }
    pantalla << "Nivel: " << nivelActual + 1 << " | Puntos: " << puntos << '\n';
    pantalla << "Controles: w (arriba), s (abajo), a (izquierda), d (derecha), q (salir)" << endl;
    cout << "\033[H\033[J" << pantalla.str();
}

void inicializarNivel() {
    auto &laberinto = niveles[nivelActual];
    fantasmas.clear();
    puntos = 0;

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

bool moverPacman(vector<string> &laberinto, int dx, int dy) {
    int nx = pacmanX + dx, ny = pacmanY + dy;

    if (laberinto[nx][ny] == PARED) return false;

    if (laberinto[nx][ny] == PUNTO || laberinto[nx][ny] == FRUTA) {
        puntos += laberinto[nx][ny] == PUNTO ? 10 : 50;
    }

    laberinto[pacmanX][pacmanY] = CAMINO;
    pacmanX = nx;
    pacmanY = ny;
    laberinto[pacmanX][pacmanY] = PACMAN;
    return true;
}

void moverFantasmas(vector<string> &laberinto) {
    for (auto &[fx, fy] : fantasmas) {
        vector<pair<int, int>> movimientos = {
            {fx - 1, fy}, {fx + 1, fy}, {fx, fy - 1}, {fx, fy + 1}
        };

        pair<int, int> mejorMovimiento = {fx, fy};
        int distanciaMinima = abs(fx - pacmanX) + abs(fy - pacmanY);

        for (auto [nx, ny] : movimientos) {
            if (laberinto[nx][ny] == CAMINO || laberinto[nx][ny] == PACMAN) {
                int distancia = abs(nx - pacmanX) + abs(ny - pacmanY);
                if (distancia < distanciaMinima) {
                    distanciaMinima = distancia;
                    mejorMovimiento = {nx, ny};
                }
            }
        }

        laberinto[fx][fy] = CAMINO;
        fx = mejorMovimiento.first;
        fy = mejorMovimiento.second;
        laberinto[fx][fy] = FANTASMA;
    }
}

bool verificarColisiones() {
    for (const auto &[fx, fy] : fantasmas) {
        if (fx == pacmanX && fy == pacmanY) return true;
    }
    return false;
}

bool verificarNivelCompletado(const vector<string> &laberinto) {
    for (const auto &fila : laberinto) {
        if (fila.find(PUNTO) != string::npos || fila.find(FRUTA) != string::npos) return false;
    }
    return true;
}

void jugar() {
    configurarTerminal();
    inicializarNivel();

    while (true) {
        imprimirLaberinto(niveles[nivelActual]);

        if (teclaPresionada()) {
            char tecla = capturarTecla();

            if (tecla == 'q') break;

            if (tecla == 'w') moverPacman(niveles[nivelActual], -1, 0);
            if (tecla == 's') moverPacman(niveles[nivelActual], 1, 0);
            if (tecla == 'a') moverPacman(niveles[nivelActual], 0, -1);
            if (tecla == 'd') moverPacman(niveles[nivelActual], 0, 1);
        }

        moverFantasmas(niveles[nivelActual]);

        if (verificarColisiones()) {
            cout << "¡Has perdido! Fin del juego." << endl;
            break;
        }

        if (verificarNivelCompletado(niveles[nivelActual])) {
            nivelActual++;
            if (nivelActual >= niveles.size()) {
                cout << "¡Has ganado el juego!" << endl;
                break;
            }
            inicializarNivel();
        }

        usleep(150000);
    }

    restaurarTerminal();
}

int main() {
    jugar();
    return 0;
}
