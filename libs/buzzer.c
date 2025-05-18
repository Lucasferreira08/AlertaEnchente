#include "buzzer.h"

#include "FreeRTOS.h"        // Kernel FreeRTOS
#include "task.h"            // API de criação e controle de tarefas FreeRTOS

#include "queue.h"

#include "global_manage.h"

#include "hardware/pwm.h"

/**
 * Configura o PWM para o buzzer
 */
void configura_pwm_buzzer(void) {
    gpio_set_function(PIN_BUZZER, GPIO_FUNC_PWM);
    uint buzzer_slice = pwm_gpio_to_slice_num(PIN_BUZZER);
    pwm_set_wrap(buzzer_slice, 62500); // Para frequência de 1kHz com clock de 125MHz
    pwm_set_chan_level(buzzer_slice, PWM_CHAN_A, 0); // Inicialmente desligado
    pwm_set_enabled(buzzer_slice, true);
}

void init_buzzer() 
{
    // Inicializa pino do buzzer
    gpio_init(PIN_BUZZER);
    gpio_set_dir(PIN_BUZZER, GPIO_OUT);
}

/**
 * Gera som no buzzer com a frequência especificada
 */
void toca_alarme(uint buzzer_slice, uint16_t freq) {
    uint32_t wrap = 125000000 / freq; // Clock de 125MHz
    pwm_set_wrap(buzzer_slice, wrap);
    pwm_set_chan_level(buzzer_slice, PWM_CHAN_A, wrap / 2); // Duty cycle de 50%
}

/**
 * Para o som do buzzer
 */
void para_alarme(uint buzzer_slice) {
    pwm_set_chan_level(buzzer_slice, PWM_CHAN_A, 0);
}

/**
 * Tarefa para controle do alerta sonoro (buzzer)
 */
void vTaskAlertaSonoro(void *pvParameters) {
    DadosSensor dados_alerta;
    uint buzzer_slice = pwm_gpio_to_slice_num(PIN_BUZZER);
    bool alarme_ativo = false;
    
    while (1) {
        // Recebe dados da fila de alertas
        if (xQueueReceive(*get_queue_alertas(), &dados_alerta, 0) == pdTRUE) {
            alarme_ativo = dados_alerta.modo_alerta;
            
            // Define o comportamento do buzzer
            if (alarme_ativo) {
                // Escolhe a frequência do alarme com base nos valores
                uint16_t freq;
                
                if (dados_alerta.nivel_agua >= NIVEL_AGUA_LIMITE && 
                    dados_alerta.volume_chuva >= VOLUME_CHUVA_LIMITE) {
                    freq = FREQ_ALERTA_AMBOS;    // Ambas condições críticas
                } else if (dados_alerta.nivel_agua >= NIVEL_AGUA_LIMITE) {
                    freq = FREQ_ALERTA_AGUA;     // Apenas nível de água crítico
                } else {
                    freq = FREQ_ALERTA_CHUVA;    // Apenas volume de chuva crítico
                }
                
                // Ativa o alarme com a frequência adequada
                toca_alarme(buzzer_slice, freq);
            } else {
                // Desativa o alarme
                para_alarme(buzzer_slice);
            }
        }
        
        // Se o alarme estiver ativo, cria um padrão de beeps
        if (alarme_ativo) {
            static bool buzzer_estado = true;
            buzzer_estado = !buzzer_estado;
            
            if (buzzer_estado) {
                uint16_t freq;
                
                if (dados_alerta.nivel_agua >= NIVEL_AGUA_LIMITE && 
                    dados_alerta.volume_chuva >= VOLUME_CHUVA_LIMITE) {
                    freq = FREQ_ALERTA_AMBOS;
                } else if (dados_alerta.nivel_agua >= NIVEL_AGUA_LIMITE) {
                    freq = FREQ_ALERTA_AGUA;
                } else {
                    freq = FREQ_ALERTA_CHUVA;
                }
                
                toca_alarme(buzzer_slice, freq);
            } else {
                para_alarme(buzzer_slice);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(DELAY_ALERTA));
    }
}