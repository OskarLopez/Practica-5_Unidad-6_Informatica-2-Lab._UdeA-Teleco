#include <iostream>
#include <vector>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cctype>

class PacmanGame {
private:
    std::vector<std::vector<char>> maze;
    int playerX, playerY;
    int score;
    bool gameOver;

    // Función para configurar entrada no bloqueante
    int kbhit() {
        struct termios oldt, newt;
        int ch;
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

        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }

        return 0;
    }

public:
    PacmanGame() : playerX(1), playerY(1), score(0), gameOver(false) {
        // Inicializar laberinto
        maze = {
            {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'},
            {'#', 'P', '.', '.', '.', '.', '.', '.', '.', '#'},
            {'#', '.', '#', '#', '.', '.', '#', '#', '.', '#'},
            {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
            {'#', '.', '#', '#', '#', '#', '#', '.', '.', '#'},
            {'#', '.', '.', '.', '.', '.', '.', '.', '.', '#'},
            {'#', '#', '#', '#', '#', '#', '#', '#', '#', '#'}
        };
    }

    void display() {
        // Mover el cursor a la posición inicial
        std::cout << "\033[H"; // Mover el cursor a la esquina superior izquierda
        for (const auto& row : maze) {
            for (char cell : row) {
                switch(cell) {
                    case '#': std::cout << "\033[1;34m#\033[0m"; break; // Paredes azules
                    case 'P': std::cout << "\033[1;33mP\033[0m"; break; // Jugador amarillo
                    case '.': std::cout << "\033[1;37m.\033[0m"; break; // Puntos blancos
                    default: std::cout << cell;
                }
                std::cout << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << "Puntuación: " << score << std::endl;
        std::cout << "Use W (arriba), A (izquierda), S (abajo), D (derecha). Q para salir." << std::endl;
    }

    void move(char direction) {
        int newX = playerX;
        int newY = playerY;

        switch(direction) {
            case 'w': newX--; break; // Mover hacia arriba
            case 's': newX++; break; // Mover hacia abajo
            case 'a': newY--; break; // Mover hacia la izquierda
            case 'd': newY++; break; // Mover hacia la derecha
            case 'q': gameOver = true; return; // Salir del juego
        }

        // Verifica si el nuevo movimiento es válido
        if (maze[newX][newY] != '#') {
            // Si hay un punto, aumenta la puntuación
            if (maze[newX][newY] == '.') {
                score += 10;
            }

            // Actualiza la posición del jugador
            maze[playerX][playerY] = '.';
            playerX = newX;
            playerY = newY;
            maze[playerX][playerY] = 'P';
        }
    }

    bool checkWin() {
        for (const auto& row : maze) {
            for (char cell : row) {
                if (cell == '.') return false; // Si hay puntos, no se ha ganado
            }
        }
        return true; // Si no hay puntos, se ha ganado
    }

    void run() {
 while (!gameOver) {
            display();
            if (kbhit()) {
                char input = std::tolower(getchar());
                move(input);
                if (checkWin()) {
                    std::cout << "¡Felicidades! Has ganado." << std::endl;
                    gameOver = true;
                }
            }
            usleep(100000); // Pausa para reducir la velocidad del juego
        }
    }
};

int main() {
    PacmanGame game;
    game.run();
    return 0;
}
