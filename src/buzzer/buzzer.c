#include <stdio.h>
#include "pico/stdlib.h"
#include "buzzer.h"
#include "hardware/pwm.h"

float freq_note = 523.25;

void play_buzzer(bool is_on)
{
    // Configura o PWM para o buzzer
    gpio_set_function(BUZZER_A_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_A_PIN);
    uint channel = pwm_gpio_to_channel(BUZZER_A_PIN);

    // Divisor de clock fixo para manter wrap dentro de 16 bits
    const float clock_divider = 8.0f;
    pwm_set_clkdiv(slice_num, clock_divider);

    // Cálculo do valor de wrap
    uint32_t wrap = (uint32_t)(125000000.0f / (clock_divider * freq_note)) - 1;
    pwm_set_wrap(slice_num, wrap);
    
    // Duty cycle de 50% para melhor qualidade de som
    // Para ajustar o volume: mude o duty cycle (ex: wrap / 4 para 25%)
    pwm_set_chan_level(slice_num, channel, wrap / 2);
    
    // Inicia o PWM
    pwm_set_enabled(slice_num, is_on);
}

void stop_buzzer()
{
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_A_PIN);
    // Desativa o PWM
    pwm_set_enabled(slice_num, false);
}