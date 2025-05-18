#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

// #include "hardware/pwm.h"

#include "FreeRTOS.h"
#include "task.h"

#include "queue.h"
// #include "semphr.h"

// Bibliotecas para periféricos
#include "ssd1306.h"   // Display OLED

#include "display.h"   // Display OLED
#include "buzzer.h"
#include "processamento.h"
#include "joystick.h"

#include "global_manage.h"


#define PIN_LED_R      16
#define PIN_LED_G      17
#define PIN_LED_B      18

int main() {
    stdio_init_all();
    
    // Inicializa periféricos
    display_init(get_ssd_pointer());
    init_buzzer();
    init_joystick();
    configura_pwm_buzzer();

    sleep_ms(10000);
    
    set_queue_sensor_date(xQueueCreate(1, sizeof(DadosSensor)));
    set_queue_dados_display(xQueueCreate(1, sizeof(DadosSensor)));
    set_queue_alertas(xQueueCreate(1, sizeof(DadosSensor)));
    
    if (*get_queue_sensor_date() == NULL || *get_queue_dados_display() == NULL || *get_queue_alertas() == NULL) {
        printf("Erro ao criar filas!\n");
        while(1); // Trava em caso de erro
    }
    
    printf("Filas criadas com sucesso.\n");
    
    // Cria as tarefas
    xTaskCreate(vJoystickTask, "LeituraJoystick", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTaskProcessamentoDados, "ProcessamentoDados", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTaskDisplayOLED, "DisplayOLED", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    // xTaskCreate(vTaskAlertaVisual, "AlertaVisual", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTaskAlertaSonoro, "AlertaSonoro", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    
    printf("Tarefas criadas com sucesso.\n");
    
    // Inicia o escalonador
    vTaskStartScheduler();
}

/**
 * Configura o hardware básico da placa
 */
void configura_hardware(void) {
    // Configura pinos do LED RGB como saída
    gpio_init(PIN_LED_R);
    gpio_init(PIN_LED_G);
    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_R, GPIO_OUT);
    gpio_set_dir(PIN_LED_G, GPIO_OUT);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
}

// /**
//  * Inicializa o display OLED
//  */
// void inicializa_display(void) {
//     display.external_vcc = false;
//     ssd1306_init(&display, OLED_WIDTH, OLED_HEIGHT, OLED_ADDR, I2C_PORT);
//     ssd1306_clear(&display);
    
//     // Exibe mensagem inicial
//     ssd1306_draw_string(&display, 0, 0, 2, "Sistema de");
//     ssd1306_draw_string(&display, 0, 20, 2, "Alerta de");
//     ssd1306_draw_string(&display, 0, 40, 2, "Enchente");
//     ssd1306_show(&display);
//     sleep_ms(2000);
//     ssd1306_clear(&display);
//     ssd1306_show(&display);
// }

/**
 * Inicializa a matriz de LEDs
 */
// void inicializa_matriz_leds(void) {
//     ht16k33_init(&matrix, I2C_PORT, MATRIX_ADDR);
//     ht16k33_clear_display(&matrix);
//     ht16k33_set_brightness(&matrix, 15); // Brilho máximo (0-15)
//     ht16k33_display_on(&matrix);
// }

//------------------------------------------------------------------------------------------------

/**
 * Define a cor do LED RGB
 */
// void set_rgb_led(uint8_t r, uint8_t g, uint8_t b) {
//     gpio_put(PIN_LED_R, r ? 1 : 0);
//     gpio_put(PIN_LED_G, g ? 1 : 0);
//     gpio_put(PIN_LED_B, b ? 1 : 0);
// }

// /**
//  * Desenha padrão normal na matriz de LEDs
//  */
// void desenha_matriz_normal(ht16k33_t *matrix) {
//     // Limpa a matriz
//     ht16k33_clear_display(matrix);
    
//     // Desenha um padrão normal (ex: um sorriso simples)
//     uint8_t sorriso[8] = {
//         0b00000000,
//         0b00100100,
//         0b00100100,
//         0b00000000,
//         0b01000010,
//         0b00111100,
//         0b00000000,
//         0b00000000
//     };
    
//     for (int i = 0; i < 8; i++) {
//         matrix->display_buffer[i] = sorriso[i];
//     }
    
//     ht16k33_update_display(matrix);
// }

// /**
//  * Desenha símbolo de alerta na matriz de LEDs
//  */
// void desenha_matriz_alerta(ht16k33_t *matrix) {
//     // Limpa a matriz
//     ht16k33_clear_display(matrix);
    
//     // Desenha um símbolo de alerta (triângulo com exclamação)
//     uint8_t alerta[8] = {
//         0b00000000,
//         0b00011000,
//         0b00111100,
//         0b00100100,
//         0b01100110,
//         0b01111110,
//         0b11111111,
//         0b00000000
//     };
    
//     for (int i = 0; i < 8; i++) {
//         matrix->display_buffer[i] = alerta[i];
//     }
    
//     ht16k33_update_display(matrix);
// }

//------------------------------------------------------------------------------------------------

/**
 * Tarefa para controle dos alertas visuais (LED RGB e matriz de LEDs)
 */
// void vTaskAlertaVisual(void *pvParameters) {
//     DadosSensor dados_alerta;
//     bool piscando = false;
    
//     while (1) {
//         // Recebe dados da fila de alertas
//         if (xQueueReceive(xQueueAlertas, &dados_alerta, 0) == pdTRUE) {
//             // Atualiza a matriz de LEDs
//             if (dados_alerta.modo_alerta) {
//                 desenha_matriz_alerta(&matrix);
//             } else {
//                 desenha_matriz_normal(&matrix);
//             }
            
//             // Define o estado do LED RGB
//             if (dados_alerta.modo_alerta) {
//                 piscando = true;
//                 // Vermelho para alerta
//                 set_rgb_led(1, 0, 0);
//             } else {
//                 piscando = false;
//                 // Verde para modo normal
//                 set_rgb_led(0, 1, 0);
//             }
//         }
        
//         // Se estiver no modo de alerta, faz o LED piscar
//         if (piscando) {
//             static bool led_estado = true;
//             led_estado = !led_estado;
            
//             if (led_estado) {
//                 set_rgb_led(1, 0, 0); // Vermelho ligado
//             } else {
//                 set_rgb_led(0, 0, 0); // Desligado
//             }
//         }
        
//         vTaskDelay(pdMS_TO_TICKS(DELAY_ALERTA / 2)); // Metade do tempo para piscar mais rápido
//     }
// }