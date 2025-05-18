#ifndef GLOBAL_MANAGE_H
#define GLOBAL_MANAGE_H

#include "FreeRTOS.h"
#include "task.h"

#include "queue.h"

// Frequências para o buzzer
#define FREQ_ALERTA_AGUA   1000
#define FREQ_ALERTA_CHUVA  1500
#define FREQ_ALERTA_AMBOS  2000

// Atraso para tarefas (ms)
#define DELAY_LEITURA      100
#define DELAY_PROCESSAMENTO 200
#define DELAY_DISPLAY      500
#define DELAY_ALERTA       1000

// Limites para modo de alerta
#define NIVEL_AGUA_LIMITE  70  // 70%
#define VOLUME_CHUVA_LIMITE 80  // 80%

#define PIN_BUZZER     15

typedef struct {
    uint8_t nivel_agua;      // 0-100%
    uint8_t volume_chuva;    // 0-100%
    bool modo_alerta;        // true: modo alerta, false: modo normal
} DadosSensor;

QueueHandle_t *get_queue_sensor_date();

QueueHandle_t *get_queue_dados_display();

QueueHandle_t *get_queue_alertas(); 

void set_queue_alertas(QueueHandle_t queue);

void set_queue_dados_display(QueueHandle_t queue);

void set_queue_sensor_date(QueueHandle_t queue);

#endif