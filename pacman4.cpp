#include <iostream>
#include <vector>
#include <unistd.h>    // Para usleep()
#include <termios.h>   // Para configuración de terminal
#include <cstdlib>     // Para system("clear")
#include <ctime>       // Para srand() y rand()
#include <sys/select.h> // Para select()

using namespace std;

// Constantes del juego
const char PARED = '#';
const char CAMINO = ' ';
const char PACMAN = 'P';
const char FANTASMA = 'G';
const char PUNTO = '.';
const char FRUTA = 'F';

// Laberintos por niveles
vector<vector<string>> niveles = {
    // Nivel 1
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

// Posiciones de Pac-Man y enemigos
int pacmanX, pacmanY;
vector<pair<int, int>> fantasmas; // Vector con las posiciones de los fantasmas
int puntos = 0;
int nivelActual = 0;

// Funciones para configurar la terminal
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

// Verifica si hay entrada disponible en el teclado
bool teclaPresionada() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = {0, 0};
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

// Captura una tecla presionada
char capturarTecla() {
    char tecla;
    read(STDIN_FILENO, &tecla, 1);
    return tecla;
}

// Imprime el laberinto y la información del juego
void imprimirLaberinto(const vector<string> &laberinto) {
    cout << "\033[H\033[J"; // Mover cursor al inicio y limpiar pantalla
    for (const auto &fila : laberinto) {
        cout << fila << endl;
    }
    cout << "Nivel: " << nivelActual + 1 << " | Puntos: " << puntos << endl;
    cout << "Controles: z (abajo), s (arriba), d (derecha), a (izquierda), q (salir)" << endl;
}

// Inicializa el nivel actual
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

// Mueve a Pac-Man
void moverPacman(vector<string> &laberinto, char direccion) {
    int nuevoX = pacmanX, nuevoY = pacmanY;

    if (direccion == 'z') nuevoX++;     // Abajo
    else if (direccion == 's') nuevoX--; // Arriba
    else if (direccion == 'd') nuevoY++; // Derecha
    else if (direccion == 'a') nuevoY--; // Izquierda

    // Teletransportación
    if (nuevoY < 0) nuevoY = static_cast<int>(laberinto[0].size()) - 1;
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

// Mueve a los fantasmas hacia Pac-Man
void moverFantasmas(vector<string> &laberinto) {
    for (auto &fantasma : fantasmas) {
        int x = fantasma.first, y = fantasma.second;
        laberinto[x][y] = CAMINO; // Limpia posición anterior

        // Movimiento del fantasma hacia Pac-Man
        int dx = pacmanX - x;
        int dy = pacmanY - y;
        int nuevoX = x, nuevoY = y;

        if (abs(dx) > abs(dy)) {
            nuevoX += (dx > 0) ? 1 : -1; // Mueve en X
        } else {
            nuevoY += (dy > 0) ? 1 : -1; // Mueve en Y
        }

        // Teletransportación
        if (nuevoY < 0) nuevoY = static_cast<int>(laberinto[0].size()) - 1;
        if (nuevoY >= static_cast<int>(laberinto[0].size())) nuevoY = 0;

        if (laberinto[nuevoX][nuevoY] == CAMINO) {
            fantasma.first = nuevoX;
            fantasma.second = nuevoY;
        }

        laberinto[fantasma.first][fantasma.second] = FANTASMA; // Actualiza posición
    }
}

// Verifica si Pac-Man colisiona con un fantasma
bool verificarColision() {
    for (const auto &fantasma : fantasmas) {
        if (pacmanX == fantasma.first && pacmanY == fantasma.second) {
            return true;
        }
    }
    return false;
}

// Verifica si el nivel ha sido completado
bool verificarVictoria(const vector<string> &laberinto) {
    for (const auto &fila : laberinto) {
        if (fila.find(PUNTO) != string::npos || fila.find(FRUTA) != string::npos) {
            return false;
        }
    }
    return true;
}

// Función principal del juego
void jugar() {
    configurarTerminal();
    while (nivelActual < static_cast<int>(niveles.size())) {
        inicializarNivel();
        vector<string> &laberinto = niveles[nivelActual];

        while (true) {
            if (teclaPresionada()) {
                char tecla = capturarTecla();
                if (tecla == 'q') {
                    restaurarTerminal();
                    cout << "¡Gracias por jugar!" << endl;
                    return;
                }
                moverPacman(laberinto, tecla);
            }

            moverFantasmas(laberinto);

            if (verificarColision()) {
                restaurarTerminal();
                cout << "¡Has sido atrapado por un fantasma! Fin del juego." << endl;
                return;
            }

            imprimirLaberinto(laberinto);

            if (verificarVictoria(laberinto)) {
                cout << "¡Nivel completado!" << endl;
                usleep(2000000); // Pausa para celebrar victoria
                break;
            }

            usleep(100000); // Espera antes de actualizar el laberinto
        }

        nivelActual++;
        if (nivelActual >= static_cast<int>(niveles.size())) {
            cout << "¡Felicidades, has completado todos los niveles!" << endl;
            break;
        }
    }

    restaurarTerminal();
    cout << "¡Juego terminado!" << endl;
}

int main() {
    srand(time(NULL)); // Inicializar generador de números aleatorios
    cout << "¡Bienvenido al juego de Pac-Man!" << endl;
    cout << "Presiona cualquier tecla para comenzar..." << endl;
    cin.get(); // Esperar a que el jugador presione Enter
    jugar();
    return 0;
}
