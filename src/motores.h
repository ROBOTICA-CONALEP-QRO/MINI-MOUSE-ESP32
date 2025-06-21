#ifndef MOTORES_H
#define MOTORES_H

// MOTOR IZQUIERDA
#define IN1 32
#define IN2 35
// MOTOR DERECHA
#define IN3 14
#define IN4 27

void inicializarMotores();
void moverAdelante();
void moverAtras();
void girarIzquierda();
void girarDerecha();
void detenerMotores();

#endif