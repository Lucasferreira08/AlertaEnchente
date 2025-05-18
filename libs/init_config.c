#include "init_config.h"

#include "hardware/adc.h"
#include "pio_matrix.pio.h"     // Programa PIO para controlar a matriz de LEDs
#include "hardware/pwm.h"       // API de PWM para controle de sinais sonoros
#include "hardware/clocks.h"    // API de clocks do RP2040

// Inicializa o PIO para a matriz de LEDs e retorna o número do state machine usado
uint pio_init(PIO pio)
{
    // Ajusta o clock do RP2040 para 128 MHz (128000 kHz)
    set_sys_clock_khz(128000, false);

    // Carrega o programa PIO na memória do PIO e retorna o offset onde ele foi colocado
    uint offset = pio_add_program(pio, &pio_matrix_program);

    // Reserva um state machine livre (bloqueante) e retorna seu índice
    uint sm = pio_claim_unused_sm(pio, true);

    // Inicializa o state machine com o programa carregado, definindo pino de saída
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    return sm;
}

// Configura o PWM no pino do buzzer para gerar o tom desejado
void pwm_init_config() 
{
    // Associa o pino do buzzer à função PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Recupera o slice de PWM correspondente ao pino
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Carrega a configuração padrão de PWM
    pwm_config config = pwm_get_default_config();
    // Ajusta o divisor de clock para que a frequência seja BUZZER_FREQUENCY com 12-bit de resolução
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    // Inicializa o slice de PWM com a configuração e já habilita o módulo
    pwm_init(slice_num, &config, true);

    // Define nível inicial zero (silêncio)
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

// Configura os pinos dos LEDs como saída
void leds_init()
{
    gpio_init(LED_GREEN);          
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
}

// Configura o PWM no pino do buzzer para gerar o tom desejado
void buzzer_pwm_config() 
{
    // Associa o pino do buzzer à função PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Recupera o slice de PWM correspondente ao pino
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Carrega a configuração padrão de PWM
    pwm_config config = pwm_get_default_config();
    // Ajusta o divisor de clock para que a frequência seja BUZZER_FREQUENCY com 12-bit de resolução
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    // Inicializa o slice de PWM com a configuração e já habilita o módulo
    pwm_init(slice_num, &config, true);

    // Define nível inicial zero (silêncio)
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void init_joystick()
{
    // Inicializa ADC para leitura do joystick
    adc_init();
    adc_gpio_init(PIN_JOYSTICK_X);
    adc_gpio_init(PIN_JOYSTICK_Y);
}