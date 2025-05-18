#include "joystick.h"

#include "FreeRTOS.h"        // Kernel FreeRTOS
#include "task.h"            // API de criação e controle de tarefas FreeRTOS

#include "queue.h"

#include "global_manage.h"

#include "hardware/adc.h"

void init_joystick()
{
    // Inicializa ADC para leitura do joystick
    adc_init();
    adc_gpio_init(PIN_JOYSTICK_X);
    adc_gpio_init(PIN_JOYSTICK_Y);
}

void vJoystickTask(void *pvParameters)
{
    DadosSensor dados;
    
    while (1) {
        // Seleciona e lê o canal do joystick X (nível de água)
        adc_select_input(0); // Canal 0 para PIN_JOYSTICK_X
        uint16_t raw_x = adc_read();
        
        // Seleciona e lê o canal do joystick Y (volume de chuva)
        adc_select_input(1); // Canal 1 para PIN_JOYSTICK_Y
        uint16_t raw_y = adc_read();
        
        // Converte para porcentagem (o ADC do RP2040 é de 12 bits, ou seja, 0-4095)
        dados.nivel_agua = (uint8_t)((raw_x * 100) / 4095);
        dados.volume_chuva = (uint8_t)((raw_y * 100) / 4095);
        
        // Determina o modo de alerta com base nos valores lidos
        dados.modo_alerta = (dados.nivel_agua >= NIVEL_AGUA_LIMITE || 
                            dados.volume_chuva >= VOLUME_CHUVA_LIMITE);
        
        // Envia os dados para a fila de processamento
        xQueueSend(*get_queue_sensor_date(), &dados, 0);
        
        // Aguarda antes da próxima leitura
        vTaskDelay(pdMS_TO_TICKS(DELAY_LEITURA));
    }
}