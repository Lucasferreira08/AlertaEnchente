#include "global_manage.h"

#include "FreeRTOS.h"        // Kernel FreeRTOS
#include "task.h"            // API de criação e controle de tarefas FreeRTOS

#include "queue.h"

// Handles de filas
QueueHandle_t xQueueDadosSensor;      // Fila para dados do sensor
QueueHandle_t xQueueDadosDisplay;     // Fila para dados a serem exibidos no display
QueueHandle_t xQueueAlertas;          // Fila para comandos de alerta

QueueHandle_t *get_queue_sensor_date() 
{
    return &xQueueDadosSensor;
}

void set_queue_sensor_date(QueueHandle_t queue) 
{
    xQueueDadosSensor=queue;
}

QueueHandle_t *get_queue_dados_display() 
{
    return &xQueueDadosDisplay;
}

void set_queue_dados_display(QueueHandle_t queue) 
{
    xQueueDadosDisplay=queue;
}

QueueHandle_t *get_queue_alertas() 
{
    return &xQueueAlertas;
}

void set_queue_alertas(QueueHandle_t queue) 
{
    xQueueAlertas=queue;
}