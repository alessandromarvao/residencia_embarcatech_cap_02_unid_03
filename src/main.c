#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "buzzer/buzzer.h"
#include "joystick/joystick.h"
#include "microphone/microphone.h"

#define BTN_A 5
#define BTN_B 6
#define JOY_BTN 22

#define R_LED_PIN 13
#define G_LED_PIN 11
#define B_LED_PIN 12

TaskHandle_t taskAHandle = NULL;
TaskHandle_t taskBHandle = NULL;
TaskHandle_t taskCHandle = NULL;
TaskHandle_t managerTaskHandle = NULL;

void taskA(void *parameter) {
    printf("Testando os LEDS...\n");

    gpio_put(R_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(400));
    gpio_put(R_LED_PIN, 0);

    gpio_put(G_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(400));
    gpio_put(G_LED_PIN, 0);

    gpio_put(B_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(400));
    gpio_put(B_LED_PIN, 0);

    printf("Testando o Buzzer...\n");
    play_buzzer(1);
    vTaskDelay(pdMS_TO_TICKS(400));
    stop_buzzer();

    printf("Pressione o botão A\n");
    while (gpio_get(BTN_A)) vTaskDelay(pdMS_TO_TICKS(10));
    printf("Botão A pressionado!\n");

    printf("Pressione o botão B\n");
    while (gpio_get(BTN_B)) vTaskDelay(pdMS_TO_TICKS(10));
    printf("Botão B pressionado!\n");

    printf("Pressione o botão SW do joystick\n");
    while (gpio_get(JOY_BTN)) vTaskDelay(pdMS_TO_TICKS(10));
    printf("Botão do joystick pressionado!\n");

    printf("Testando o microfone...\n");
    adc_select_input(MIC_CHN);
    uint16_t valor_mic;
    while ((valor_mic = adc_read()) < 1250) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    printf("Microfone funcionando. Valor: %d\n", valor_mic);
    vTaskDelay(pdMS_TO_TICKS(500));

    xTaskNotifyGive(managerTaskHandle);
    vTaskDelete(NULL);
}

void taskB(void *parameter) {
    printf("Executando taskB: LED vermelho piscando\n");
    printf("Acende o LED vermelho..\n");
    gpio_put(R_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    printf("Apaga o LED vermelho..\n");
    gpio_put(R_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(500));

    xTaskNotifyGive(managerTaskHandle);
    vTaskDelete(NULL);
}

void taskC(void *parameter) {
    adc_select_input(ADC_X);
    uint16_t valor_x = adc_read();
    adc_select_input(ADC_Y);
    uint16_t valor_y = adc_read();

    float tensao_x = (valor_x * 3.3) / 4095;
    float tensao_y = (valor_y * 3.3) / 4095;



    printf("Joystick X: %.2f V, Valor recebido: %d V\n", tensao_x, valor_x);

    printf("Joystick Y: %.2f V, Valor recebido: %d V\n", tensao_y, valor_y);
    
    if ((tensao_x > 3.0) || (tensao_y > 3.0)) {
        play_buzzer(true);
    } else {
        play_buzzer(false);
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    xTaskNotifyGive(managerTaskHandle);
    vTaskDelete(NULL);
}

void managerTask(void *parameter) {
    for (;;) {
        xTaskCreate(taskA, "TaskA", 2048, NULL, 1, &taskAHandle);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        xTaskCreate(taskB, "TaskB", 1024, NULL, 1, &taskBHandle);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        xTaskCreate(taskC, "TaskC", 1024, NULL, 1, &taskCHandle);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (1) {
            xTaskCreate(taskC, "TaskC", 1024, NULL, 1, &taskCHandle);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }
}

int main() {
    stdio_init_all();

    gpio_init(R_LED_PIN); gpio_set_dir(R_LED_PIN, GPIO_OUT); gpio_put(R_LED_PIN, 0);
    gpio_init(G_LED_PIN); gpio_set_dir(G_LED_PIN, GPIO_OUT); gpio_put(G_LED_PIN, 0);
    gpio_init(B_LED_PIN); gpio_set_dir(B_LED_PIN, GPIO_OUT); gpio_put(B_LED_PIN, 0);

    gpio_init(BTN_A); gpio_set_dir(BTN_A, GPIO_IN); gpio_pull_up(BTN_A);
    gpio_init(BTN_B); gpio_set_dir(BTN_B, GPIO_IN); gpio_pull_up(BTN_B);
    gpio_init(JOY_BTN); gpio_set_dir(JOY_BTN, GPIO_IN); gpio_pull_up(JOY_BTN);

    joystick_init();
    init_microphone();

    while (!stdio_usb_connected());

    xTaskCreate(managerTask, "ManagerTask", 2048, NULL, 2, &managerTaskHandle);

    vTaskStartScheduler();

    while (true);
}
