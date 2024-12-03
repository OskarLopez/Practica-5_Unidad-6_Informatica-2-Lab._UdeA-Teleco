#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <climits>
#include <vector>
#include <stdexcept>

const int WIDTH = 31;
const int HEIGHT = 20;

class PacmanGame {
private:
    char maze[HEIGHT][WIDTH];
    int pacmanX, pacmanY;
    int ghostX, ghostY;
    int fruits;

    // Capturar una sola pulsación de tecla sin esperar a que se introduzca la información
    char getch() {
        struct termios oldt, newt;
        char ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }

    void initializeMaze() {
        const char* mazeLayout[HEIGHT] = {
            "###############################",
            "#.......#....#.....#........#.",
            "#.#.###.#.#.#.#.#.#.#######.#.",
            "#.#...#.#...#.#.#.#.#.....#.#.",
            "#.###.#.#####.#.#.#.#.###.#.#.",
            "#.P....#........#G#.#.#...#.#.",
            "#.#####.#######.#.#.#.#####.#.",
            "#......#........#.#.#......#.",
            "#.#####.#######.#.#.#######.#.",
            "#.#...#.#...#...#.#.#.....#.#.",
            "#.#.#.#.#.#.###.#.#.#.#.#.#.#.",
            "#.#.#.#.#.#.#...#.#.#.#.#.#.#.",
            "#.#.#.#.#.#.#.#.#.#.#.#.#.#.#.",
            "#.#...#.#...#.#.#.#.#...#.#.#.",
            "#.#########.#.#.#.#.#####.#.#.",
            "#.#........#.#.#.#.#......#.#.",
            "#.#.#######.#.#.#.#.#######.#.",
            "#.#.#.....#.#.#.#.#.#......#.",
            "#.#.#.###.#.#.#.#.#.#.#####.#.",
            "###############################"
        };

        // Copiar de forma segura el diseño del laberinto
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH - 1; j++) {
                maze[i][j] = mazeLayout[i][j];
            }
            maze[i][WIDTH - 1] = '\0';  // Null-terminate each row
        }
    }

public:
    PacmanGame() {
        srand(time(0));
        restartGame();
    }

    void showIntroduction() {
        system("clear");
        std::cout << "**************************************************" << std::endl;
        std::cout << "*            Bienvenido al Juego de Pacman     *" << std::endl;
        std::cout << "*        Desarrollado por Oscar Lopez Peñata   *" << std::endl;
        std::cout << "*          Estudiante de Telecomunicaciones    *" << std::endl;
        std::cout << "*           Universidad de Antioquia           *" << std::endl;
        std::cout << "*        Presentado al profesor Johnny         *" << std::endl;
        std::cout << "*        Alexander Aguirre para la asignatura  *" << std::endl;
        std::cout << "*               Informática 2 Laboratorio      *" << std::endl;
        std::cout << "**************************************************" << std::endl;
        std::cout << "Instrucciones:" << std::endl;
        std::cout << "- Usa 'w', 'a', 's', 'd' para mover a Pacman." << std::endl;
        std::cout << "- Evita al fantasma y recoge las frutas." << std::endl;
        std::cout << "- Presiona 'q' para salir o 'r' para reiniciar." << std::endl;
        std::cout << std::endl << "Presiona cualquier tecla para comenzar..." << std::endl;
        getch();
    }

    void printMaze() {
        system("clear");
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH - 1; j++) {
                std::cout << maze[i][j];
            }
            std::cout << std::endl;
        }
        std::cout << "Pacman: (" << pacmanX << ", " << pacmanY << ")" << std::endl;
        std::cout << "Fantasma: (" << ghostX << ", " << ghostY << ")" << std::endl;
        std::cout << "Frutas recolectadas: " << fruits << std::endl;
    }

    void movePacman(char move) {
        // Comprobación de límites
        if (pacmanY < 0 || pacmanY >= HEIGHT || pacmanX < 0 || pacmanX >= WIDTH - 1) {
            return;
        }

        maze[pacmanY][pacmanX] = ' ';
        
        // Validar y mover Pacman según la dirección del movimiento
        try {
            if (move == 'w' && pacmanY > 0 && maze[pacmanY - 1][pacmanX] != '#') pacmanY--;
            else if (move == 's' && pacmanY < HEIGHT - 1 && maze[pacmanY + 1][pacmanX] != '#') pacmanY++;
            else if (move == 'a' && pacmanX > 0 && maze[pacmanY][pacmanX - 1] != '#') pacmanX--;
            else if (move == 'd' && pacmanX < WIDTH - 2 && maze[pacmanY][pacmanX + 1] != '#') pacmanX++;
        } catch (const std::out_of_range& e) {
            // Manejar posibles errores fuera de rango
            return;
        }

        // Recoger fruta
        if (maze[pacmanY][pacmanX] == '.') {
            fruits++;
        }

        maze[pacmanY][pacmanX] = 'P';
    }

    void moveGhost() {
        // Comprobación de límites
        if (ghostY < 0 || ghostY >= HEIGHT || ghostX < 0 || ghostX >= WIDTH - 1) {
            return;
        }

        maze[ghostY][ghostX] = ' ';
        int newX = ghostX, newY = ghostY;
        
        // Simple IA fantasma para perseguir a Pacman
        int dx = (pacmanX > ghostX) ? 1 : ((pacmanX < ghostX) ? -1 : 0);
        int dy = (pacmanY > ghostY) ? 1 : ((pacmanY < ghostY) ? -1 : 0);

        // Intenta moverte directamente hacia Pacman.
        if (ghostY + dy >= 0 && ghostY + dy < HEIGHT && 
            ghostX + dx >= 0 && ghostX + dx < WIDTH - 1 && 
            maze[ghostY + dy][ghostX + dx] != '#') {
            newY += dy;
            newX += dx;
        } else {
            // Si la ruta directa está bloqueada, pruebe direcciones alternativas
            int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
            int bestDist = INT_MAX;
            
            for (auto& dir : directions) {
                int x = ghostX + dir[0];
                int y = ghostY + dir[1];
                
                if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH - 1 && maze[y][x] != '#') {
                    int dist = abs(pacmanX - x) + abs(pacmanY - y);
                    if (dist < bestDist) {
                        bestDist = dist;
                        newX = x;
                        newY = y;
                    }
                }
            }
        }

        ghostX = newX;
        ghostY = newY;
        maze[ghostY][ghostX] = 'G';
    }

    void restartGame() {
        // Inicializar el estado del juego
        initializeMaze();
        
        // Posiciones iniciales predeterminadas
        pacmanX = 5;
        pacmanY = 5;
        ghostX = 25;
        ghostY = 15;
        fruits = 0;

        // Reiniciar el laberinto con Pacman y Ghost
        maze[pacmanY][pacmanX] = 'P';
        maze[ghostY][ghostX] = 'G';
    }

    bool play() {
        showIntroduction();

        bool gameRunning = true;
        while (gameRunning) {
            printMaze();
            char move = getch();

            if (move == 'q') {
                gameRunning = false;
            } else if (move == 'r') {
                restartGame();
            } else {
                movePacman(move);
                moveGhost();

                // Comprueba si el juego ha terminado
                if (pacmanX == ghostX && pacmanY == ghostY) {
                    printMaze();
                    std::cout << "¡Game Over! Pacman fue atrapado por el fantasma." << std::endl;
                    std::cout << "Frutas recolectadas: " << fruits << std::endl;
                    std::cout << "Presiona 'r' para reiniciar o 'q' para salir." << std::endl;

                    while (true) {
                        char restart = getch();
                        if (restart == 'r') {
                            restartGame();
                            break;
                        } else if (restart == 'q') {
                            gameRunning = false;
                            break;
                        }
                    }
                }
            }
        }

        return true;
    }
};

int main() {
    PacmanGame game;
    game.play();
    return 0;
}
