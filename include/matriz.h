#ifndef MATRIZ_H  // Se INIT_CONFIG_H não estiver definido
#define MATRIZ_H  // Defina INIT_CONFIG_H

#include "pio_matrix.pio.h"

// Initializes the keypad
uint32_t matrix_rgb(double b, double r, double g);

// Checks if a specific key is pressed
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b);

// Função para exibir o semáforo na matriz de LEDs
void desenhar_alerta(PIO pio, uint sm);

// Função para exibir o semáforo na matriz de LEDs
void apagar_matriz(PIO pio, uint sm);

#endif