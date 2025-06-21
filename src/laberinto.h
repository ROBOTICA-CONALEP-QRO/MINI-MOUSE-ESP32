#ifndef LABERINTO_H
#define LABERINTO_H
#include "proximitySensors.h"

#define MAZE_WIDTH 10
#define MAZE_HEIGHT 10
// Tamaño de cada celda en mm
#define CELL_SIZE 180 // 180 mm = 18 cm
// tamaño del robot 10cm
// Umbral para detectar si esta en medio de una celda
#define CELL_THRESHOLD 55
// UMBRAL para detectar si hay una pared
#define WALL_THRESHOLD 70

// Movimiento - ms
#define forwardTime 500 //ms
// Distancia aproximada que recorrer en 500ms
#define forwardDistance 200 //mm 



typedef enum {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
} Orientation;

// Accioón actual del robot
typedef enum {
    NOTHING = 0,
    ROTATE = 1,
    MOVE_FORWARD = 2,
    MOVE_BACKWARD = 3,
} Action;

typedef struct {
    bool visited;
    bool wallNorth;
    bool wallEast;
    bool wallSouth;
    bool wallWest;
} Cell;

typedef struct {
    int x;
    int y;
    Orientation dir;
    Action currentAction;
} RobotState;

typedef struct {
    int frontCells;
    int leftCells;
    int rightCells;
} CellCounts;

extern Cell maze[MAZE_WIDTH][MAZE_HEIGHT];
extern RobotState robot;

void initializeMaze(void *parameters); // Tarea FreeRTOS para inicializar el laberinto
void updateCellWalls(int x, int y, bool frontWall, bool rightWall, bool leftWall);

void moveRobotLeft();
void moveRobotRight();
void moveRobotForward();
void moveRobotBackward();

void decideNextMove();
CellCounts calcCells(ProximityData data);
void updateNewCell(bool frontWall, bool rightWall, bool leftWall);

#endif