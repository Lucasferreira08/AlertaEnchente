#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"       // API de PWM para controle de sinais sonoros
#include "hardware/clocks.h"    // API de clocks do RP2040

// #include "hardware/pwm.h"

#include "FreeRTOS.h"
#include "task.h"

#include "queue.h"
// #include "semphr.h"

// Bibliotecas para periféricos
#include "ssd1306.h"   // Display OLED
#include "global_data.h"   // Display OLED
#include "init_config.h"   // Display OLED
#include "matriz.h"   // Display OLED

#define PIN_JOYSTICK_X 26    // ADC0
#define PIN_JOYSTICK_Y 27    // ADC1

// Atraso para tarefas (ms)
#define DELAY_LEITURA      100
#define DELAY_PROCESSAMENTO 100
#define DELAY_DISPLAY      100
#define DELAY_ALERTA       20


// #define PIN_LED_R      16
// #define PIN_LED_G      17
// #define PIN_LED_B      18

ssd1306_t ssd;

QueueHandle_t xQueueDadosSensor;      // Fila para dados do sensor
QueueHandle_t xQueueDadosDisplay;     // Fila para dados a serem exibidos no display
QueueHandle_t xQueueAlertas;          // Fila para comandos de alerta
QueueHandle_t xQueueBuzzer;          // Fila para comandos de alerta

/**
 * Tarefa para atualização do display OLED
 */
void vDisplayTask(void *pvParameters) {
    DadosSensor dados_display;

    display_init(&ssd);
    
    while (1) {

        printf("Display Task \n");

        // Recebe dados da fila de display
        if (xQueueReceive(xQueueDadosDisplay, &dados_display, portMAX_DELAY) == pdTRUE) {
            printf("Entrou no if - Display \n");
            // Atualiza o display de acordo com o modo de operação
            if (dados_display.modo_alerta) {
                //desenha_display_alerta(get_ssd_pointer(), &dados_display);
                printf("Display Task - desenho\n");
                desenha_display_alerta(&ssd, &dados_display);
                printf("Desenho 1\n");
            } else {
                printf("Display Task - desenho 2\n");
                desenha_display_normal(&ssd, &dados_display);
                printf("Desenho 2\n");
            }
        }
        
        printf("Display Task - delay\n");
        vTaskDelay(pdMS_TO_TICKS(DELAY_DISPLAY));
    }
}

void vProcessamentoTask(void *pvParameters) {
    DadosSensor dados_recebidos;
    
    while (1) {
        // Recebe dados da fila de leitura
        if (xQueueReceive(xQueueDadosSensor, &dados_recebidos, portMAX_DELAY) == pdTRUE) {
            // Aplica qualquer processamento adicional aqui se necessário
            
            // Envia os dados processados para as filas de display e alertas
            xQueueSend(xQueueDadosDisplay, &dados_recebidos, 0);
            xQueueSend(xQueueAlertas, &dados_recebidos, 0);
            xQueueSend(xQueueBuzzer, &dados_recebidos, 0);
            
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

void vJoystickTask(void *pvParameters)
{
    DadosSensor dados;

    init_joystick();
    
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
        xQueueSend(xQueueDadosSensor, &dados, 0);
        
        // Aguarda antes da próxima leitura
        vTaskDelay(pdMS_TO_TICKS(DELAY_LEITURA));
    }
}

/**
 * Tarefa para controle dos alertas visuais (LED RGB e matriz de LEDs)
 */
void vAlertaTask(void *pvParameters) {
    DadosSensor dados_alerta;
    // bool piscando = false;
    PIO pio = pio0;            // Escolhe a instância PIO 0
    uint sm = pio_init(pio);   // Inicializa o state machine e retorna qual SM foi usado
    
    while (1) {
        // Recebe dados da fila de alertas
        if (xQueueReceive(xQueueAlertas, &dados_alerta, 0) == pdTRUE) {
            // Atualiza a matriz de LEDs
            if (dados_alerta.modo_alerta) {
                desenhar_alerta(pio, sm);
            } else {
                apagar_matriz(pio, sm);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(DELAY_ALERTA / 2)); // Metade do tempo para piscar mais rápido
    }
}

void vBuzzerTask(void *pvParameters) {
    DadosSensor dados_alerta;
    buzzer_pwm_config();
    leds_init();
    bool ativo;
    
    while (1) {
        // Recebe dados da fila de alertas
        if (xQueueReceive(xQueueBuzzer, &dados_alerta, 0) == pdTRUE) {
            if (dados_alerta.modo_alerta) {
                ativo = true;
            } else {
                ativo = false;
            }
        }

        if (ativo) 
        {
            gpio_put(LED_RED, 0);
            gpio_put(LED_BLUE, 1);
            pwm_set_gpio_level(BUZZER_PIN, 2048);
            vTaskDelay(pdMS_TO_TICKS(200));

            gpio_put(LED_BLUE, 0);
            gpio_put(LED_RED, 1);
            pwm_set_gpio_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_put(LED_RED, 0);
        }
        else 
        {
            gpio_put(LED_BLUE, 0);
            gpio_put(LED_RED, 0);
            pwm_set_gpio_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        
        //vTaskDelay(pdMS_TO_TICKS(DELAY_ALERTA));
    }
}

int main() {
    stdio_init_all();
    
    xQueueDadosSensor = xQueueCreate(1, sizeof(DadosSensor));
    xQueueDadosDisplay = xQueueCreate(1, sizeof(DadosSensor));
    xQueueAlertas = xQueueCreate(1, sizeof(DadosSensor));
    xQueueBuzzer = xQueueCreate(1, sizeof(DadosSensor));

    // set_queue_alertas(xQueueCreate(1, sizeof(DadosSensor)));
    
    if (xQueueDadosSensor == NULL || xQueueDadosDisplay == NULL || xQueueAlertas == NULL || xQueueBuzzer == NULL) {  
        printf("Erro ao criar filas!\n");
        while(1); // Trava em caso de erro
    }
    
    printf("Filas criadas com sucesso.\n");
    
    // Cria as tarefas
    xTaskCreate(vJoystickTask, "LeituraJoystick", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vProcessamentoTask, "ProcessamentoDados", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vDisplayTask, "DisplayOLED", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vAlertaTask, "AlertaVisual", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vBuzzerTask, "AlertaBuzzer", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    // xTaskCreate(vTaskAlertaSonoro, "AlertaSonoro", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    
    printf("Tarefas criadas com sucesso.\n");
    
    // Inicia o escalonador
    vTaskStartScheduler();

    panic_unsupported();
}