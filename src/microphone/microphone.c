#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "microphone.h"

/**
 * Inicializa o ADC do microfone
 */
void init_microphone()
{
    // Set up the ADC to read from the microphone pin
    adc_init();
    adc_gpio_init(MIC_PIN);
}