#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

class PacmanGame {
private:
    std::vector<std::vector<char>> maze; // Laberinto
    int playerX, playerY; // Posición del jugador
    int ghostX, ghostY;   // Posición del fantasma
    int score;            // Puntuación del jugador
    bool isInvulnerable;  // Estado de invulnerabilidad
    int invulnerableTime; // Tiempo de invulnerabilidad

    char getKeyPress() {
        struct termios oldt, newt;
        char ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        return ch;
    }

    bool isGhostAtPosition(int x, int y) const {
        return x == ghostX && y == ghostY;
    }

public:
    PacmanGame() : playerX(1), playerY(1), ghostX(3), ghostY(3), score(0), isInvulnerable(false), invulnerableTime(0) {
        maze = {
            {'#', '#', '#', '#', '#'},
            {'#', '.', '.', '@', '#'},
            {'#', '.', '#', '.', '#'},
            {'#', '.', '.', '.', '#'},
            {'#', '#', '#', '#', '#'}
        };
    }

    void display() {
        std::cout << "\033[2J\033[H"; // Limpia la pantalla y mueve el cursor al inicio
        for (size_t i = 0; i < maze.size(); ++i) {
            for (size_t j = 0; j < maze[i].size(); ++j) {
                if (static_cast<int>(i) == playerX && static_cast<int>(j) == playerY) {
                    std::cout << "\033[1;33mP\033[0m "; // Jugador (amarillo)
                } else if (isGhostAtPosition(static_cast<int>(i), static_cast<int>(j))) {
                    std::cout << (isInvulnerable ? "\033[1;32mG\033[0m " : "\033[1;31mG\033[0m "); // Fantasma (rojo o verde si es invulnerable)
                } else {
                    switch (maze[i][j]) {
                        case '#': std::cout << "\033[1;34m#\033[0m "; break; // Pared (azul)
                        case '.': std::cout << "\033[1;37m.\033[0m "; break; // Punto (blanco)
                        case '@': std::cout << "\033[1;35m@\033[0m "; break; // Píldora (morado)
                        default: std::cout << "  "; // Espacio vacío
                    }
                }
            }
            std::cout << std::endl;
        }

        std::cout << "Puntuación: " << score << "\tInvulnerable: " << (isInvulnerable ? "Sí" : "No") << std::endl;
        std::cout << "Use W (arriba), A (izquierda), S (abajo), D (derecha). Q para salir." << std::endl;
    }

    void movePlayer(char direction) {
        int newX = playerX, newY = playerY;

        switch (direction) {
            case 'w': newX--; break; // Arriba
            case 's': newX++; break; // Abajo
            case 'a': newY--; break; // Izquierda
            case 'd': newY++; break; // Derecha
            default: return; // Tecla inválida
        }

        if (maze[newX][newY] != '#') { // Si no es una pared
            playerX = newX;
            playerY = newY;

            if (maze[playerX][playerY] == '.') { // Si es un punto
                score++;
                maze[playerX][playerY] = ' '; // Limpia el punto
            } else if (maze[playerX][playerY] == '@') { // Si es una píldora
                isInvulnerable = true; // Activa la invulnerabilidad
                invulnerableTime = 10; // Duración de la invulnerabilidad
                score += 5; // Aumenta la puntuación
                maze[playerX][playerY] = ' '; // Limpia la píldora
            }
        }
    }

    void moveGhost() {
        if (!isInvulnerable) {
            int direction = rand() % 4; // Genera un movimiento aleatorio
            int newX = ghostX, newY = ghostY;

            switch (direction) {
                case 0: newX--; break; // Arriba
                case 1: newX++; break; // Abajo
                case 2: newY--; break; // Izquierda
                case 3: newY++; break; // Derecha
            }

            if (maze[newX][newY] != '#' && !isGhostAtPosition(newX, newY)) { // Si no es una pared y no hay otro fantasma
                ghostX = newX;
                ghostY = newY;
            }
        }
    }

    void update() {
        if (isInvulnerable) {
            invulnerableTime--;
            if (invulnerableTime <= 0) {
                isInvulnerable = false; // Desactiva la invulnerabilidad
            }
        }
    }

    bool isGameOver() const {
        return !isInvulnerable && playerX == ghostX && playerY == ghostY;
    }

    void run() {
        srand(time(0)); // Inicializa el generador de números aleatorios
        char input;

        while (!isGameOver()) {
            display();
            input = getKeyPress();
            if (input == 'q') break; // Salir del juego
            movePlayer(input);
            moveGhost();
            update(); // Actualiza el estado del juego
            usleep(200000); // Pausa para hacer el juego jugable (200 ms)
        }

        if (isGameOver()) {
            std::cout << "¡El fantasma te atrapó! Fin del juego." << std::endl;
        } else {
            std::cout << "Saliste del juego." << std::endl;
        }
    }
};

// Función principal
int main() {
    PacmanGame game;
    game.run();
    return 0;
}
