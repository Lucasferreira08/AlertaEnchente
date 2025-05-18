#ifndef DISPLAY_H
#define DISPLAY_H

#include "global_manage.h"

#include "ssd1306.h"

void display_init(ssd1306_t *ssd); 

void vTaskDisplayOLED(void *pvParameters);

#endif