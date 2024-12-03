#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QKeyEvent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scene(new QGraphicsScene(this)) {

    ui->setupUi(this);
    ui->graphicsView->setFixedSize(400, 400);
    scene->setSceneRect(0, 0, 400, 400);
    ui->graphicsView->setScene(scene);

    createMaze();    // Crea los rectángulos del laberinto
    createDots();    // Crea los puntos comestibles
    createPlayer();  // Crea el personaje

    setFocus(); // Establece el foco para capturar las teclas
    scoreLabel = new QLabel(this);
    livesLabel = new QLabel(this);

    scoreLabel->setGeometry(10, 10, 100, 30);
    livesLabel->setGeometry(70, 10, 100, 30);
    updateLabels();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::createMaze() {
    // Dibujando rectángulos del laberinto (muros)
    QList<QRect> mazeWalls = {
        QRect(50, 50, 300, 10),
        QRect(50, 340, 300, 10),
        QRect(50, 50, 10, 300),
        QRect(340, 50, 10, 300),
        QRect(150, 50, 10, 150),
        QRect(250, 100, 10, 150),
        QRect(100, 200, 200, 10),
        QRect(150, 250, 10, 100),
        QRect(250, 280, 10, 70)
    };

    for (const QRect &rect : mazeWalls) {
        QGraphicsRectItem *wall = new QGraphicsRectItem(rect);
        wall->setBrush(Qt::blue); // Color azul para los muros
        scene->addItem(wall);
        walls.append(wall); // Añade los muros a la lista de paredes
    }
}

void MainWindow::createDots() {
    // Crear puntos comestibles en áreas libres del laberinto
    QList<QPoint> dotPositions = {
        {70, 70}, {90, 70}, {110, 70}, {130, 70}, {170, 70}, {210, 70}, {230, 70}, {270, 70},
        {70, 110}, {270, 110},
        {70, 150}, {130, 150}, {210, 150}, {270, 150},
        {70, 190}, {130, 190}, {170, 190}, {210, 190}, {270, 190},
        {70, 290}, {130, 290}, {170, 290}, {210, 290}, {270, 290},
        {70, 330}, {90, 330}, {110, 330}, {130, 330}, {170, 330}, {210, 330}, {230, 330}, {270, 330}
    };

    for (const QPoint &pos : dotPositions) {
        QGraphicsEllipseItem *dot = new QGraphicsEllipseItem(pos.x(), pos.y(), 5, 5);
        dot->setBrush(Qt::yellow); // Color amarillo para los puntos
        scene->addItem(dot);
        dots.append(dot); // Guarda los puntos en una lista para detectar colisiones
    }
}

void MainWindow::createPlayer() {
    // Crear el personaje como un círculo rojo
    player = new QGraphicsEllipseItem(70, 70, 10, 10);
    player->setBrush(Qt::red); // Color rojo para el personaje
    scene->addItem(player);
}

// Función para capturar eventos de teclado y mover el personaje
/*void MainWindow::keyPressEvent(QKeyEvent *event) {
    int dx = 0;
    int dy = 0;

    // Cambiar la posición del personaje según la tecla presionada
    switch (event->key()) {
    case Qt::Key_Up:
        dy = -10;
        break;
    case Qt::Key_Down:
        dy = 10;
        break;
    case Qt::Key_Left:
        dx = -10;
        break;
    case Qt::Key_Right:
        dx = 10;
        break;
    default:
        return;
    }

    // Nueva posición del personaje
    int newX = player->x() + dx;
    int newY = player->y() + dy;

    // Verifica si la nueva posición colisiona con algún muro
    bool collision = false;
    for (QGraphicsRectItem *wall : walls) {
        if (player->collidesWithItem(wall)) {  // Verifica colisión con el muro
            collision = true;
            break;
        }
    }

    // Si no hay colisión, mueve el personaje
    if (!collision) {
        player->moveBy(dx, dy);  // Mueve el personaje usando moveBy para aplicar dx y dy
    }

    // Detectar si el personaje come algún punto
    for (int i = 0; i < dots.size(); ++i) {
        if (player->collidesWithItem(dots[i])) {
            scene->removeItem(dots[i]); // Remueve el punto de la escena
            dots.removeAt(i);           // Remueve el punto de la lista
            break;
        }
    }
}*/
void MainWindow::updateLabels() {
    scoreLabel->setText("Puntos: " + QString::number(score));
    livesLabel->setText("Vidas: " + QString::number(lives));
}
void MainWindow::keyPressEvent(QKeyEvent *event) {
    int dx = 0;
    int dy = 0;

    // Cambiar la posición del personaje según la tecla presionada
    switch (event->key()) {
    case Qt::Key_Up:
        dy = -10;
        break;
    case Qt::Key_Down:
        dy = 10;
        break;
    case Qt::Key_Left:
        dx = -10;
        break;
    case Qt::Key_Right:
        dx = 10;
        break;
    default:
        return;
    }

    // Temporariamente mueve el personaje para verificar si colisiona
    player->moveBy(dx, dy);

    // Verifica si la nueva posición del personaje colisiona con algún muro
    bool collision = false;
    for (QGraphicsRectItem *wall : walls) {
        if (player->collidesWithItem(wall)) {
            collision = true;
            break;
        }
    }

    // Si hay colisión, resta una vida y deshacer el movimiento
    if (collision) {
        player->moveBy(-dx, -dy);
        lives--;
        updateLabels();
        if (lives == 0) {
            // Reiniciar juego si las vidas llegan a 0
            QMessageBox::information(this, "Juego Terminado", "Perdiste todas las vidas");
            lives = 3;
            score = 0;
            updateLabels();
        }
        return;
    }

    // Detectar si el personaje come algún punto
    for (int i = 0; i < dots.size(); ++i) {
        if (player->collidesWithItem(dots[i])) {
            scene->removeItem(dots[i]); // Remueve el punto de la escena
            dots.removeAt(i);           // Remueve el punto de la lista
            score++;
            updateLabels();
            break;
        }
    }
}
