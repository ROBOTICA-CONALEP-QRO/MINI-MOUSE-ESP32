#ifndef MOTORES_H
#define MOTORES_H

// MOTOR IZQUIERDA
#define IN1 2
#define IN2 3
// MOTOR DERECHA
#define IN3 4
#define IN4 5

void inicializarMotores();
void moverAdelante();
void moverAtras();
void girarIzquierda();
void girarDerecha();
void detenerMotores();

#endif