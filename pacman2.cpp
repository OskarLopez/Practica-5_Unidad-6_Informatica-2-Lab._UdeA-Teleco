#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

class PacmanGame {
private:
    std::vector<std::vector<char>> maze; // Laberinto actual
    int playerX, playerY;                // Posición del jugador
    std::vector<std::pair<int, int>> ghosts; // Posiciones de los fantasmas
    int score;                           // Puntuación
    int lives;                           // Vidas
    bool levelComplete;                  // Indica si el nivel se completó

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
        for (const auto &ghost : ghosts) {
            if (ghost.first == x && ghost.second == y) return true;
        }
        return false;
    }

    void moveGhosts() {
        for (auto &ghost : ghosts) {
            int direction = rand() % 4; // Movimiento aleatorio
            int newX = ghost.first, newY = ghost.second;

            switch (direction) {
                case 0: newX--; break; // Arriba
                case 1: newX++; break; // Abajo
                case 2: newY--; break; // Izquierda
                case 3: newY++; break; // Derecha
            }

            if (maze[newX][newY] != '#' && !isGhostAtPosition(newX, newY)) {
                ghost.first = newX;
                ghost.second = newY;
            }
        }
    }

    bool isLevelCompleted() {
        for (const auto &row : maze) {
            for (char cell : row) {
                if (cell == '.') return false; // Si quedan puntos, no está completo
            }
        }
        return true;
    }

    void loadMaze(int level) {
        if (level == 1) {
            maze = {
                {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
                {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
                {'#', '.', '#', '#', '#', '.', '#', '#', '.', '#'},
                {'#', '.', '.', '.', '#', '.', '#', '.', '.', '#'},
                {'#', '.', '#', '.', '.', '.', '.', '#', '.', '#'},
                {'#', '.', '#', '#', '#', '.', '#', '#', '.', '#'},
                {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
                {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
            };
            playerX = 1; playerY = 1;
            ghosts = {{6, 8}, {2, 2}};
        } else if (level == 2) {
            maze = {
                {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
                {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
                {'#', '.', '#', '#', '#', '.', '#', '#', '#', '#', '.', '#'},
                {'#', '.', '.', '.', '#', '.', '.', '.', '.', '#', '.', '#'},
                {'#', '#', '#', '.', '#', '#', '#', '#', '.', '#', '.', '#'},
                {'#', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
                {'#', '.', '#', '#', '#', '#', '.', '#', '#', '#', '.', '#'},
                {'#', '.', '.', '.', '#', '.', '.', '.', '#', '.', '.', '#'},
                {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
            };
            playerX = 1; playerY = 1;
            ghosts = {{7, 10}, {3, 3}, {5, 5}};
        }
    }

public:
    PacmanGame() : score(0), lives(3), levelComplete(false) {}

    void display() {
        std::cout << "\033[2J\033[H"; // Limpia la pantalla
        for (size_t i = 0; i < maze.size(); ++i) {
            for (size_t j = 0; j < maze[i].size(); ++j) {
                if (static_cast<int>(i) == playerX && static_cast<int>(j) == playerY) {
                    std::cout << "\033[1;33mP\033[0m "; // Jugador
                } else if (isGhostAtPosition(static_cast<int>(i), static_cast<int>(j))) {
                    std::cout << "\033[1;31mG\033[0m "; // Fantasma
                } else {
                    switch (maze[i][j]) {
                        case '#': std::cout << "\033[1;34m#\033[0m "; break; // Pared
                        case '.': std::cout << "\033[1;37m.\033[0m "; break; // Punto
                        default: std::cout << "  "; // Espacio vacío
                    }
                }
            }
            std::cout << std::endl;
        }
        std::cout << "Puntuación: " << score << " | Vidas: " << lives << std::endl;
    }

    void movePlayer(char direction) {
        int newX = playerX, newY = playerY;

        switch (direction) {
            case 'w': newX--; break; // Arriba
            case 's': newX++; break; // Abajo
            case 'a': newY--; break; // Izquierda
            case 'd': newY++; break; // Derecha
            default: return;
        }

        if (maze[newX][newY] != '#') {
            playerX = newX;
            playerY = newY;

            if (maze[playerX][playerY] == '.') {
                score++;
                maze[playerX][playerY] = ' ';
            }
        }
    }

    void run() {
        int level = 1;
        srand(time(0));
        while (true) {
            loadMaze(level);
            levelComplete = false;

            while (!levelComplete && lives > 0) {
                display();
                char input = getKeyPress();
                movePlayer(input);
                moveGhosts();

                if (isGhostAtPosition(playerX, playerY)) {
                    lives--;
                    if (lives == 0) {
                        display();
                        std::cout << "¡Perdiste todas tus vidas! Fin del juego." << std::endl;
                        return;
                    }
                }

                levelComplete = isLevelCompleted();
                usleep(200000);
            }

            if (levelComplete) {
                std::cout << "¡Completaste el nivel " << level << "!" << std::endl;
                level++;
                if (level > 2) {
                    std::cout << "¡Felicidades, completaste todos los niveles!" << std::endl;
                    return;
                }
            }
        }
    }
};

int main() {
    PacmanGame game;
    game.run();
    return 0;
}
