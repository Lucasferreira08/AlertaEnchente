#include "display.h"

#include "FreeRTOS.h"        // Kernel FreeRTOS
#include "task.h"            // API de criação e controle de tarefas FreeRTOS

#include "queue.h"

#include "hardware/i2c.h"

void display_init(ssd1306_t *ssd) 
{
    i2c_init(I2C_PORT, 400 * 1000);
 
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line

    ssd1306_init(ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(ssd);                                         // Configura o display
    ssd1306_send_data(ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

// void desenha_display_normal2(ssd1306_t *display) {
//     char buffer[30];
    
//     ssd1306_fill(display, 0);
    
//     // Nível de água
//     ssd1306_draw_string(display, "Nivel de agua:", 0, 0);
//     // snprintf(buffer, sizeof(buffer), "%d%%", dados->nivel_agua);
//     // ssd1306_draw_string(display, buffer, 0, 10);
    
//     // Volume de chuva
//     ssd1306_draw_string(display, "Volume chuva:", 0, 20);
//     // snprintf(buffer, sizeof(buffer), "%d%%", dados->volume_chuva);
//     // ssd1306_draw_string(display, buffer, 0, 30);
    
//     ssd1306_send_data(display);
// }

/**
 * Tarefa para atualização do display OLED
 */
void vTaskDisplayOLED(void *pvParameters) {
    DadosSensor dados_display;
    
    while (1) {

        printf("Display Task \n");

        // Recebe dados da fila de display
        if (xQueueReceive(*get_queue_dados_display(), &dados_display, portMAX_DELAY) == pdTRUE) {
            printf("Entrou no if - Display \n");
            // Atualiza o display de acordo com o modo de operação
            if (dados_display.modo_alerta) {
                //desenha_display_alerta(get_ssd_pointer(), &dados_display);
                printf("Display Task - desenho\n");
                //desenha_display_normal(get_ssd_pointer(), &dados_display);
                printf("Desenho 1\n");
            } else {
                printf("Display Task - desenho 2\n");
                //desenha_display_normal(get_ssd_pointer(), &dados_display);
                printf("Desenho 2\n");
            }
        }
        
        printf("Display Task - delay\n");
        vTaskDelay(pdMS_TO_TICKS(DELAY_DISPLAY));
    }
}