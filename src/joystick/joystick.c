#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "joystick.h"

void joystick_init()
{
    // Inicializa o módulo ADC do RP2040
    adc_init();

    // Configura os pinos GPIO26 e GPIO27 como entradas analógicas
    adc_gpio_init(VRY); // VRy → ADC0
    adc_gpio_init(VRX); // VRx → ADC1
}