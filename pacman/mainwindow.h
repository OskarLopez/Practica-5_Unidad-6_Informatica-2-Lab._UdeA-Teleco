#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QKeyEvent>
#include <QLabel> // Para mostrar vidas y puntaje

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override; // Captura de teclas para el movimiento

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QGraphicsEllipseItem *player; // Personaje principal

    // Listas de paredes y puntos para la detección de colisiones
    QList<QGraphicsRectItem *> walls;
    QList<QGraphicsEllipseItem *> dots;

    // Variables de estado del juego
    int lives = 3;      // Contador de vidas
    int score = 0;      // Contador de puntos
    QLabel *scoreLabel; // Etiqueta para mostrar el puntaje
    QLabel *livesLabel; // Etiqueta para mostrar las vidas


    // Métodos para crear los elementos del tablero
    void createMaze();   // Crea los muros del laberinto
    void createDots();   // Crea los puntos comestibles
    void createPlayer(); // Crea el personaje
    void updateLabels(); // Actualiza los indicadores de vidas y puntaje
};

#endif // MAINWINDOW_H
