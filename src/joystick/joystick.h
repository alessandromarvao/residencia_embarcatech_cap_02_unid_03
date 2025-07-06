#ifndef JOYSTICK_H
#define JOYSTICK_H

#define VRY 26
#define VRX 27

// --- Mapeamento dos canais ADC utilizados ---
#define ADC_Y 0 // Canal ADC 0 corresponde ao GPIO26 (eixo Y do joystick)
#define ADC_X 1 // Canal ADC 1 corresponde ao GPIO27 (eixo X do joystick)

void joystick_init();

#endif