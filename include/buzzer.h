#ifndef BUZZER_H
#define BUZZER_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

void init_buzzer();

void toca_alarme(uint buzzer_slice, uint16_t freq);

void para_alarme(uint buzzer_slice);

void vTaskAlertaSonoro(void *pvParameters);

void configura_pwm_buzzer(void);

#endif