#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"      // Biblioteca padrão para I/O e inicialização do Pico
#include "hardware/adc.h"     // API do conversor ADC para leitura de tensões
#include "hardware/pwm.h"     // API de PWM para controle de sinais sonoros
#include "hardware/clocks.h"  // API de clocks do RP2040 para gerenciar frequências

#include "FreeRTOS.h"         // Kernel FreeRTOS
#include "task.h"             // API para criação e gerenciamento de tarefas
#include "queue.h"            // API para criação e uso de filas de mensagens

// Bibliotecas específicas do projeto
#include "ssd1306.h"          // Driver para display OLED SSD1306
#include "global_data.h"      // Definições de tipo e constantes de dados compartilhados
#include "init_config.h"      // Funções de inicialização de periféricos e configurações
#include "matriz.h"           // Controle de matriz de LEDs para alertas visuais

// Definições de pinos para joystick (ADC)
#define PIN_JOYSTICK_X 26    // ADC0 para eixo X (nível de água)
#define PIN_JOYSTICK_Y 27    // ADC1 para eixo Y (volume de chuva)

// Delays em milissegundos para cada tarefa
#define DELAY_LEITURA      100
#define DELAY_PROCESSAMENTO 100
#define DELAY_DISPLAY      100
#define DELAY_ALERTA       20

ssd1306_t ssd;              // Estrutura que representa o display OLED

// Filas de comunicação entre tarefas
QueueHandle_t xQueueDadosSensor;      // Leitura bruta do joystick
QueueHandle_t xQueueDadosDisplay;     // Dados a serem mostrados no OLED
QueueHandle_t xQueueAlertas;          // Dados para alerta visual (LEDs)
QueueHandle_t xQueueBuzzer;           // Dados para alerta sonoro (buzzer)

/**
 * Tarefa para atualização do display OLED.
 * Recebe dados na fila xQueueDadosDisplay e atualiza a tela.
 */
void vDisplayTask(void *pvParameters) {
    DadosSensor dados_display;

    // Inicializa o display SSD1306
    display_init(&ssd);
    
    while (1) {
        // Debug: sinaliza entrada na tarefa
        printf("Display Task: aguardando dados...\n");

        // Aguarda dados de display indefinidamente
        if (xQueueReceive(xQueueDadosDisplay, &dados_display, portMAX_DELAY) == pdTRUE) {
            // Se estiver modo alerta, chama função específica
            if (dados_display.modo_alerta) {
                desenha_display_alerta(&ssd, &dados_display);
            } else {
                // Caso normal, desenho padrão
                desenha_display_normal(&ssd, &dados_display);
            }
        }
        // Pequeno delay para liberar CPU
        vTaskDelay(pdMS_TO_TICKS(DELAY_DISPLAY));
    }
}

/**
 * Tarefa de processamento de dados.
 * Recebe dados brutos do joystick, aplica lógica e distribui para outras filas.
 */
void vProcessamentoTask(void *pvParameters) {
    DadosSensor dados_recebidos;
    
    while (1) {
        // Aguarda dados do leitor de joystick
        if (xQueueReceive(xQueueDadosSensor, &dados_recebidos, portMAX_DELAY) == pdTRUE) {
            // Aqui poderia inserir filtros ou calibrações adicionais
            
            // Envia dados processados para display, alertas visuais e sonoros
            xQueueSend(xQueueDadosDisplay, &dados_recebidos, 0);
            xQueueSend(xQueueAlertas, &dados_recebidos, 0);
            xQueueSend(xQueueBuzzer, &dados_recebidos, 0);
            
            // Debug: imprime valores no console
            printf("Dados processados: Nivel=%d%%, Chuva=%d%%, Modo=%s\n",
                   dados_recebidos.nivel_agua,
                   dados_recebidos.volume_chuva,
                   dados_recebidos.modo_alerta ? "ALERTA" : "Normal");
        }
        // Delay para tolerância de processamento
        vTaskDelay(pdMS_TO_TICKS(DELAY_PROCESSAMENTO));
    }
}

/**
 * Tarefa de leitura do joystick via ADC.
 * Converte as leituras em percentuais e verifica condição de alerta.
 */
void vJoystickTask(void *pvParameters)
{
    DadosSensor dados;

    // Inicializa ADC e pinos de joystick
    init_joystick();
    
    while (1) {
        // Lê eixo X (nível de água)
        adc_select_input(0);
        uint16_t raw_x = adc_read();
        
        // Lê eixo Y (volume de chuva)
        adc_select_input(1);
        uint16_t raw_y = adc_read();
        
        // Converte valores brutos (0-4095) para porcentagem (0-100)
        dados.nivel_agua      = (uint8_t)((raw_x * 100) / 4095);
        dados.volume_chuva    = (uint8_t)((raw_y * 100) / 4095);
        
        // Define se está em modo alerta baseado em limites predefinidos
        dados.modo_alerta = (dados.nivel_agua >= NIVEL_AGUA_LIMITE ||
                             dados.volume_chuva >= VOLUME_CHUVA_LIMITE);
        
        // Envia struct para fila de processamento
        xQueueSend(xQueueDadosSensor, &dados, 0);
        
        // Aguarda próximo ciclo de leitura
        vTaskDelay(pdMS_TO_TICKS(DELAY_LEITURA));
    }
}

/**
 * Tarefa para controle de alertas visuais via PIO e matriz de LEDs.
 */
void vAlertaTask(void *pvParameters) {
    DadosSensor dados_alerta;
    PIO pio = pio0;
    uint sm = pio_init(pio); // Inicializa State Machine para PIO
    
    while (1) {
        // Se receber dados, atualiza matriz
        if (xQueueReceive(xQueueAlertas, &dados_alerta, 0) == pdTRUE) {
            if (dados_alerta.modo_alerta) {
                desenhar_alerta(pio, sm);
            } else {
                apagar_matriz(pio, sm);
            }
        }
        // Delay reduzido para piscar mais rápido
        vTaskDelay(pdMS_TO_TICKS(DELAY_ALERTA / 2));
    }
}

/**
 * Tarefa para controle do buzzer e LEDs de indicação sonora.
 */
void vBuzzerTask(void *pvParameters) {
    DadosSensor dados_alerta;
    bool ativo = false;

    // Configura PWM para buzzer e inicializa LEDs
    buzzer_pwm_config();
    leds_init();
    
    while (1) {
        // Atualiza flag 'ativo' conforme dados da fila
        if (xQueueReceive(xQueueBuzzer, &dados_alerta, 0) == pdTRUE) {
            ativo = dados_alerta.modo_alerta;
        }

        if (ativo) {
            // Em modo alerta, alterna LED e gera tom no buzzer
            gpio_put(LED_RED, 0);
            gpio_put(LED_BLUE, 1);
            pwm_set_gpio_level(BUZZER_PIN, 2048);
            vTaskDelay(pdMS_TO_TICKS(200));

            gpio_put(LED_BLUE, 0);
            gpio_put(LED_RED, 1);
            pwm_set_gpio_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_put(LED_RED, 0);
        } else {
            // Desliga ambos se não alerta
            gpio_put(LED_BLUE, 0);
            gpio_put(LED_RED, 0);
            pwm_set_gpio_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

/**
 * Função principal: inicializa filas, tarefas e inicia o escalonador FreeRTOS.
 */
int main() {
    // Inicia todas as interfaces de I/O do Pico (UART, USB, etc.)
    stdio_init_all();
    
    // Cria filas para troca de mensagens entre tarefas
    xQueueDadosSensor   = xQueueCreate(1, sizeof(DadosSensor));
    xQueueDadosDisplay  = xQueueCreate(1, sizeof(DadosSensor));
    xQueueAlertas       = xQueueCreate(1, sizeof(DadosSensor));
    xQueueBuzzer        = xQueueCreate(1, sizeof(DadosSensor));

    // Verifica sucesso na criação das filas
    if (!xQueueDadosSensor || !xQueueDadosDisplay || !xQueueAlertas || !xQueueBuzzer) {
        printf("Erro ao criar filas!\n");
        while (1); // Trava em caso de falha crítica
    }
    
    printf("Filas criadas com sucesso.\n");
    
    // Cria tarefas com prioridades e pilhas mínimas
    xTaskCreate(vJoystickTask,    "LeituraJoystick",    configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vProcessamentoTask, "ProcessamentoDados", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vDisplayTask,     "DisplayOLED",        configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vAlertaTask,      "AlertaVisual",       configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vBuzzerTask,      "AlertaBuzzer",       configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    
    printf("Tarefas criadas com sucesso.\n");
    
    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    // Caso o escalonador retorne, indica plataforma não suportada
    panic_unsupported();
}
