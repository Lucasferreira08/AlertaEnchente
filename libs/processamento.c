#include "processamento.h"

#include "FreeRTOS.h"        // Kernel FreeRTOS
#include "task.h"            // API de criação e controle de tarefas FreeRTOS

#include "queue.h"

#include "global_manage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

/**
 * Tarefa para processamento dos dados dos sensores
 */
void vTaskProcessamentoDados(void *pvParameters) {
    DadosSensor dados_recebidos;
    
    while (1) {
        // Recebe dados da fila de leitura
        if (xQueueReceive(*get_queue_sensor_date(), &dados_recebidos, portMAX_DELAY) == pdTRUE) {
            // Aplica qualquer processamento adicional aqui se necessário
            
            // Envia os dados processados para as filas de display e alertas
            xQueueSend(*get_queue_dados_display(), &dados_recebidos, 0);
            xQueueSend(*get_queue_alertas(), &dados_recebidos, 0);
            
            // Para fins de depuração
            printf("Dados processados: Nível: %d%%, Chuva: %d%%, Modo: %s\n", 
                   dados_recebidos.nivel_agua, 
                   dados_recebidos.volume_chuva, 
                   dados_recebidos.modo_alerta ? "ALERTA" : "Normal");
        }
        
        // Pequeno atraso para não sobrecarregar o processador
        vTaskDelay(pdMS_TO_TICKS(DELAY_PROCESSAMENTO));
    }
}