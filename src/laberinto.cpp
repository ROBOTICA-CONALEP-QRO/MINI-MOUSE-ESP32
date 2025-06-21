#include "laberinto.h"
#include <Arduino.h>
#include "proximitySensors.h"
#include "motores.h"
#include "giroscopio.h"
#include "lineDetector.h"

Cell maze[MAZE_WIDTH][MAZE_HEIGHT];
RobotState robot;
CellCounts lastCellCounts = {0, 0, 0};
int ciclosAvanzados = 0;


void initializeMaze(void *parameters)
{
    Serial.println("Inicializando laberinto...");
    for (int x = 0; x < MAZE_WIDTH; x++)
    {
        for (int y = 0; y < MAZE_HEIGHT; y++)
        {
            maze[x][y].visited = false;
            maze[x][y].wallNorth = true;
            maze[x][y].wallEast = true;
            maze[x][y].wallSouth = true;
            maze[x][y].wallWest = true;
        }
    }

    robot.x = 0;
    robot.y = 0;
    robot.dir = NORTH; // Iniciar mirando al norte
    robot.currentAction = NOTHING;
    maze[robot.x][robot.y].visited = true;

    Serial.println("Laberinto inicializado.");
    resetData();
    while (true)
    {
        // Esperar indefinidamente
        decideNextMove();
        vTaskDelay(forwardTime / portTICK_PERIOD_MS);
    }
}

void updateCellWalls(int x, int y, bool frontWall, bool rightWall, bool leftWall)
{
    if (x < 0 || x >= MAZE_WIDTH || y < 0 || y >= MAZE_HEIGHT)
    {
        Serial.println("Error: Coordenadas fuera de rango al actualizar paredes.");
        return;
    }
    switch (robot.dir)
    {
    case NORTH:
        maze[x][y].wallNorth = frontWall;
        maze[x][y].wallEast = rightWall;
        maze[x][y].wallWest = leftWall;
        break;
    case EAST:
        maze[x][y].wallEast = frontWall;
        maze[x][y].wallSouth = rightWall;
        maze[x][y].wallNorth = leftWall;
        break;
    case SOUTH:
        maze[x][y].wallSouth = frontWall;
        maze[x][y].wallWest = rightWall;
        maze[x][y].wallEast = leftWall;
        break;
    case WEST:
        maze[x][y].wallWest = frontWall;
        maze[x][y].wallNorth = rightWall;
        maze[x][y].wallSouth = leftWall;
        break;
    default:
        Serial.println("Error: Dirección del robot no válida al actualizar paredes.");
        return;
    }
    // Actualizar las paredes de la celda adyacente
    switch (robot.dir)
    {
    case NORTH:
        if (y > 0)
            maze[x][y - 1].wallSouth = frontWall;
        break;
    case EAST:
        if (x < MAZE_WIDTH - 1)
            maze[x + 1][y].wallWest = frontWall;
        break;
    case SOUTH:
        if (y < MAZE_HEIGHT - 1)
            maze[x][y + 1].wallNorth = frontWall;
        break;
    case WEST:
        if (x > 0)
            maze[x - 1][y].wallEast = frontWall;
        break;
    default:
        Serial.println("Error: Dirección del robot no válida al actualizar paredes.");
        return;
    }
}

void moveRobotLeft()
{
    robot.currentAction = ROTATE;
    float targetAngle = 90.0f;
    resetData();
    float currentAngle = GiroscopioData().angleZ;
    while (abs(currentAngle - targetAngle) > 5.0f)
    {
        girarIzquierda();
        currentAngle = GiroscopioData().angleZ;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    detenerMotores();
    robot.currentAction = NOTHING;
    robot.dir = (Orientation)((robot.dir + 3) % 4);
}
void moveRobotRight()
{
    robot.currentAction = ROTATE;
    float targetAngle = -90.0f;
    resetData();
    float currentAngle = GiroscopioData().angleZ;
    while (abs(currentAngle - targetAngle) > 5.0f)
    {
        girarDerecha();
        currentAngle = GiroscopioData().angleZ;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    detenerMotores();
    robot.currentAction = NOTHING;
    robot.dir = (Orientation)((robot.dir + 1) % 4);
}
void rotate180()
{
    robot.currentAction = ROTATE;
    float targetAngle = 180.0f;
    resetData();
    float currentAngle = GiroscopioData().angleZ;
    while (abs(currentAngle - targetAngle) > 5.0f)
    {
        girarDerecha();
        currentAngle = GiroscopioData().angleZ;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    detenerMotores();
    robot.currentAction = NOTHING;
    robot.dir = (Orientation)((robot.dir + 2) % 4);
}

void moveRobotForward()
{
    robot.currentAction = MOVE_FORWARD;
    resetData();
    moverAdelante();
}
void moveRobotBackward()
{
    robot.currentAction = MOVE_BACKWARD;
    resetData();
    moverAtras();
}

/**
 * Función para decidir el siguiente movimiento del robot
 */
void decideNextMove()
{
    // Obtener datos de los sensores de proximidad
    ProximityData data = getProximityData();

    if (robot.currentAction != ROTATE)
    {
        // Calcular celdas detectadas por los sensores
        CellCounts counts = calcCells(data);
        
        // Verificar si el robot cambió de celda (comparando con mediciones anteriores)
        if (counts.frontCells < lastCellCounts.frontCells)
        {
            // Actualizar posición del robot según su dirección
            switch (robot.dir)
            {
            case NORTH:
                robot.y++;
                break;
            case EAST:
                robot.x++;
                break;
            case SOUTH:
                robot.y--;
                break;
            case WEST:
                robot.x--;
                break;
            }
            // Marcar la nueva celda como visitada
            maze[robot.x][robot.y].visited = true;
            
            // Actualizar información de paredes en la nueva celda
            updateNewCell(data.frontDistance <= WALL_THRESHOLD, 
                          data.rightDistance <= WALL_THRESHOLD, 
                          data.leftDistance <= WALL_THRESHOLD);
            
            Serial.printf("Nueva celda: (%d,%d), Dir: %d\n", robot.x, robot.y, robot.dir);
        }
        
        // Guardar los conteos actuales para la próxima iteración
        lastCellCounts = counts;
        
        // Camino despejado al frente (distancia >= 1260mm)
        if (data.frontDistance >= 1260)
        {
            // Continuar avanzando
            moveRobotForward();
            ciclosAvanzados++;
            
            // Calcular si ha recorrido una celda completa
            int distanciaAproximada = forwardDistance * ciclosAvanzados;
            if (distanciaAproximada > CELL_SIZE)
            {
                // Calcular cuántas celdas completas ha recorrido
                int celdasRecorridas = distanciaAproximada / CELL_SIZE;
                
                // Actualizar la posición del robot
                switch (robot.dir)
                {
                case NORTH:
                    robot.y += celdasRecorridas;
                    break;
                case EAST:
                    robot.x += celdasRecorridas;
                    break;
                case SOUTH:
                    robot.y -= celdasRecorridas;
                    break;
                case WEST:
                    robot.x -= celdasRecorridas;
                    break;
                }
                
                // Actualizar información de paredes en la nueva celda
                updateNewCell(data.frontDistance <= WALL_THRESHOLD, 
                              data.rightDistance <= WALL_THRESHOLD, 
                              data.leftDistance <= WALL_THRESHOLD);
                
                // Marcar la nueva celda como visitada
                maze[robot.x][robot.y].visited = true;
                
                // Reiniciar el contador de ciclos avanzados
                ciclosAvanzados = 0;
                
                Serial.printf("Avance múltiple: (%d,%d), Dir: %d\n", robot.x, robot.y, robot.dir);
            }
        }
        // Obstáculo al frente pero no inmediato (distancia < 1260mm)
        else if (data.frontDistance > WALL_THRESHOLD && data.frontDistance <= 1260)
        {
            if (ciclosAvanzados > 0)
            {
                // Si el robot ha avanzado, actualizar la posición
                int distanciaAproximada = forwardDistance * ciclosAvanzados;
                if (distanciaAproximada > CELL_SIZE)
                {
                    int celdasRecorridas = distanciaAproximada / CELL_SIZE;
                    if (celdasRecorridas > 0)
                    {
                        switch (robot.dir)
                        {
                        case NORTH:
                            robot.y += celdasRecorridas;
                            break;
                        case EAST:
                            robot.x += celdasRecorridas;
                            break;
                        case SOUTH:
                            robot.y -= celdasRecorridas;
                            break;
                        case WEST:
                            robot.x -= celdasRecorridas;
                            break;
                        }
                        
                        // Actualizar información de paredes
                        updateNewCell(data.frontDistance <= WALL_THRESHOLD, 
                                     data.rightDistance <= WALL_THRESHOLD, 
                                     data.leftDistance <= WALL_THRESHOLD);
                        
                        // Marcar la nueva celda como visitada
                        maze[robot.x][robot.y].visited = true;
                        
                        Serial.printf("Actualización parcial: (%d,%d), Dir: %d\n", robot.x, robot.y, robot.dir);
                    }
                    ciclosAvanzados = 0;
                }
            }
            
            // Decidir hacia dónde girar - prioridad al camino no visitado
            bool izquierdaVisitada = false;
            bool derechaVisitada = false;
            
            // Determinar si las celdas adyacentes ya fueron visitadas
            switch (robot.dir)
            {
            case NORTH:
                if (robot.x > 0) izquierdaVisitada = maze[robot.x-1][robot.y].visited;
                if (robot.x < MAZE_WIDTH-1) derechaVisitada = maze[robot.x+1][robot.y].visited;
                break;
            case EAST:
                if (robot.y > 0) derechaVisitada = maze[robot.x][robot.y-1].visited;
                if (robot.y < MAZE_HEIGHT-1) izquierdaVisitada = maze[robot.x][robot.y+1].visited;
                break;
            case SOUTH:
                if (robot.x < MAZE_WIDTH-1) izquierdaVisitada = maze[robot.x+1][robot.y].visited;
                if (robot.x > 0) derechaVisitada = maze[robot.x-1][robot.y].visited;
                break;
            case WEST:
                if (robot.y < MAZE_HEIGHT-1) derechaVisitada = maze[robot.x][robot.y+1].visited;
                if (robot.y > 0) izquierdaVisitada = maze[robot.x][robot.y-1].visited;
                break;
            }
            
            // Si hay espacio a ambos lados, priorizar el camino no visitado
            if (data.leftDistance > WALL_THRESHOLD && data.rightDistance > WALL_THRESHOLD) {
                if (!izquierdaVisitada && derechaVisitada) {
                    moveRobotLeft();
                    Serial.println("Girando izquierda (camino no visitado)");
                } else if (izquierdaVisitada && !derechaVisitada) {
                    moveRobotRight();
                    Serial.println("Girando derecha (camino no visitado)");
                }
                // Si ambos caminos están visitados o no visitados, elegir el que tenga más distancia
                else if (data.leftDistance > data.rightDistance) {
                    moveRobotLeft();
                    Serial.println("Girando izquierda (mayor distancia)");
                } else {
                    moveRobotRight();
                    Serial.println("Girando derecha (mayor distancia)");
                }
            }
            // Si solo hay espacio a un lado, ir por ahí
            else if (data.leftDistance > WALL_THRESHOLD) {
                moveRobotLeft();
                Serial.println("Girando izquierda (único camino)");
            } else if (data.rightDistance > WALL_THRESHOLD) {
                moveRobotRight();
                Serial.println("Girando derecha (único camino)");
            }
            // Si no hay salida, avanzar hasta detectar mejor la situación
            else {
                moveRobotForward();
                Serial.println("Avanzando para evaluar mejor");
            }
        }
        // Pared muy cercana al frente (distancia <= WALL_THRESHOLD)
        else if (data.frontDistance <= WALL_THRESHOLD)
        {
            // Detener motores ante una pared cercana
            detenerMotores();
            
            // Actualizar información de paredes
            updateNewCell(true, data.rightDistance <= WALL_THRESHOLD, data.leftDistance <= WALL_THRESHOLD);
            
            // Decidir hacia dónde girar
            if (data.leftDistance > WALL_THRESHOLD && data.rightDistance > WALL_THRESHOLD) {
                // Si ambos lados están libres, verificar si alguno está no visitado
                bool izquierdaVisitada = false;
                bool derechaVisitada = false;
                
                // Determinar si las celdas adyacentes están visitadas
                switch (robot.dir)
                {
                case NORTH:
                    if (robot.x > 0) izquierdaVisitada = maze[robot.x-1][robot.y].visited;
                    if (robot.x < MAZE_WIDTH-1) derechaVisitada = maze[robot.x+1][robot.y].visited;
                    break;
                case EAST:
                    if (robot.y > 0) derechaVisitada = maze[robot.x][robot.y-1].visited;
                    if (robot.y < MAZE_HEIGHT-1) izquierdaVisitada = maze[robot.x][robot.y+1].visited;
                    break;
                case SOUTH:
                    if (robot.x < MAZE_WIDTH-1) izquierdaVisitada = maze[robot.x+1][robot.y].visited;
                    if (robot.x > 0) derechaVisitada = maze[robot.x-1][robot.y].visited;
                    break;
                case WEST:
                    if (robot.y < MAZE_HEIGHT-1) derechaVisitada = maze[robot.x][robot.y+1].visited;
                    if (robot.y > 0) izquierdaVisitada = maze[robot.x][robot.y-1].visited;
                    break;
                }
                
                // Priorizar camino no visitado
                if (!izquierdaVisitada && derechaVisitada) {
                    moveRobotLeft();
                    Serial.println("Girando izquierda (no visitado)");
                } else if (izquierdaVisitada && !derechaVisitada) {
                    moveRobotRight();
                    Serial.println("Girando derecha (no visitado)");
                } else if (data.leftDistance > data.rightDistance) {
                    moveRobotLeft();
                    Serial.println("Girando izquierda (mayor espacio)");
                } else {
                    moveRobotRight();
                    Serial.println("Girando derecha (mayor espacio)");
                }
            } else if (data.leftDistance > WALL_THRESHOLD) {
                moveRobotLeft();
                Serial.println("Girando izquierda (única opción)");
            } else if (data.rightDistance > WALL_THRESHOLD) {
                moveRobotRight();
                Serial.println("Girando derecha (única opción)");
            } else {
                // Callejón sin salida, girar 180 grados
                rotate180();
                Serial.println("Rotando 180° (sin salida)");
            }
        }
    }
}

CellCounts calcCells(ProximityData data)
{
    CellCounts counts = {0, 0, 0};

    if (data.frontDistance <= 1266)
    {
        counts.frontCells = data.frontDistance / CELL_SIZE;
    }
    if (data.leftDistance <= 1266)
    {
        counts.leftCells = data.leftDistance / CELL_SIZE;
    }
    if (data.rightDistance <= 1266)
    {
        counts.rightCells = data.rightDistance / CELL_SIZE;
    }

    return counts;
}

void updateNewCell(bool frontWall, bool rightWall, bool leftWall)
{
    // Actualizar las paredes de la celda actual
    updateCellWalls(robot.x, robot.y, frontWall, rightWall, leftWall);
}