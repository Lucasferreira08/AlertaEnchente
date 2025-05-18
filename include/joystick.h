#ifndef JOYSTICK_H
#define JOYSTICK_H

// Definições de pinos
#define PIN_JOYSTICK_X 26    // ADC0
#define PIN_JOYSTICK_Y 27    // ADC1

void vJoystickTask(void *pvParameters);
void init_joystick();

#endif