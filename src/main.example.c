/*
================================================================================
Exemplo prático 1: Tarefa envia dados lidos do ADC, outra atua sobre eles
(Item 4.7 do E-Book)
=======================================================
*/

// --- Inclusão das bibliotecas essenciais do Pico SDK e do FreeRTOS ---
#include <stdio.h>
#include "pico/stdlib.h"  // Funções de I/O e temporização do Raspberry Pi Pico
#include "hardware/adc.h" // Controle do ADC interno (para leitura analógica)
#include "FreeRTOS.h"     // Núcleo do FreeRTOS
#include "task.h"         // Funções de criação e controle de tarefas
#include "queue.h"        // Funções de criação e manipulação de filas
#include "buzzer/buzzer.h"
#include "joystick/joystick.h"
#include "microphone/microphone.h"

#define BTN_A 5
#define BTN_B 6
#define JOY_BTN 22

// --- Pinos de saída digital usados para acionar os LEDs ---
#define R_LED_PIN 13 // LED Vermelho
#define G_LED_PIN 11 // LED Verde
#define B_LED_PIN 12 // LED Azul

// --- Estrutura de dados para armazenar as leituras do joystick ---
typedef struct
{
    uint16_t x; // Valor do eixo X (0 a 4095)
    uint16_t y; // Valor do eixo Y (0 a 4095)
} JoystickData_t;

// --- Declaração da fila usada para comunicação entre as tarefas ---
QueueHandle_t fila_joystick;

bool estado_anterior_btn = false;

bool is_btn_pressionado(uint8_t botao)
{
    if (botao && !estado_anterior_btn)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// --- Tarefa responsável por inicializar os componentes ---
void tarefa_self_test(void *param)
{
    // --- Testar LEDs RGB: acender e apagar sequencialmente ---
    // LED Vermelho:
    printf("Testando os LEDS...\n");
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("Ligando LED vermelho...\n");
    gpio_put(R_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(400));
    printf("Desligando LED vermelho...\n");
    gpio_put(R_LED_PIN, 0);

    // LED Verde:
    printf("Ligando LED verde...\n");
    gpio_put(G_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(400));
    printf("Desligando LED verde...\n");
    gpio_put(G_LED_PIN, 0);

    // Led Azul
    printf("Ligando LED azul...\n");
    gpio_put(B_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(400));
    printf("Desligando LED azul...\n");
    gpio_put(B_LED_PIN, 0);

    // --- Testar Buzzer: gerar som simples ---
    printf("Testando o Buzzer...\n");
    vTaskDelay(pdMS_TO_TICKS(500));
    play_buzzer(400);

    // --- Testar botões A, B e joystick SW ---
    printf("Testando os botões...\n");
    vTaskDelay(pdMS_TO_TICKS(500));
    printf("Pressione o botão A\n");
    while (gpio_get(BTN_A))
        ;
    printf("Botão A pressionado!\n");

    printf("Pressione o botão B\n");
    while (gpio_get(BTN_B))
        ;
    printf("Botão B pressionado!\n");

    printf("Pressione o botão SW do joystick\n");
    while (gpio_get(JOY_BTN))
        ;
    printf("Botão do joystick pressionado!\n");

    printf("Testando o microfone...\n");
    adc_select_input(MIC_CHN);
    uint16_t valor_mic;

    while (adc_read() < 1200)
    {
        valor_mic = adc_read();
        printf("Valor recebido: %d\n", valor_mic);
    }
    printf("Microfone funcionando. Valor recebido: %d\n", valor_mic);

    vTaskDelete(NULL);
}

// --- Tarefa que aciona o LED vermelho ---
void tarefa_alive_task(void *param)
{
    printf("Acionando o LED vermelho...\n");
    gpio_put(R_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_put(R_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
}

// --- Tarefa que controla os movimentos do Joystick
void tarefa_monitor_joystick_alarme(void *param)
{
    while (true)
    {
        // Seleciona o canal do eixo X e realiza a leitura
        adc_select_input(ADC_X);
        uint16_t valor_x = adc_read();

        // Seleciona o canal do eixo Y e realiza a leitura
        adc_select_input(ADC_Y);
        uint16_t valor_y = adc_read();

        float tensao_x = (valor_x * 3.3) / 4095;
        float tensao_y = (valor_y * 3.3) / 4095;
        printf("Leitura do eixo X do Joystick: %1f V (valor obtido do ADC: %d).\n", tensao_x, valor_x);
        printf("Leitura do eixo Y do Joystick: %1f V (valor obtido do ADC: %d).\n", tensao_y, valor_y);
        vTaskDelay(pdMS_TO_TICKS(50));

        // Liga o G_LED_PIN se o valor do eixo X for maior que 3000
        gpio_put(G_LED_PIN, (tensao_x > 3.0));

        // Liga o R_LED_PIN se o valor do eixo Y for maior que 3000
        gpio_put(R_LED_PIN, (tensao_y > 3.0));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    printf("Stack overflow em tarefa %s\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

// --- Função principal do programa ---
int main()
{

    // Inicializa a comunicação serial padrão (para debug, se necessário)
    stdio_init_all();

    // Inicializa os pinos dos LEDs e configura como saída digital
    gpio_init(R_LED_PIN);
    gpio_set_dir(R_LED_PIN, GPIO_OUT);
    gpio_init(G_LED_PIN);
    gpio_set_dir(G_LED_PIN, GPIO_OUT);
    gpio_init(B_LED_PIN);
    gpio_set_dir(B_LED_PIN, GPIO_OUT);

    // Desliga todos os LEDs ao inicializar a aplicação
    gpio_put(R_LED_PIN, 0);
    gpio_put(G_LED_PIN, 0);
    gpio_put(B_LED_PIN, 0);

    // Inicializa os Botões
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    gpio_init(JOY_BTN);
    gpio_set_dir(JOY_BTN, GPIO_IN);
    gpio_pull_up(JOY_BTN);

    // Inicializa o ADC dos Joysticks
    joystick_init();

    // Inicializa o ADC do microfone
    init_microphone();

    // Aguarda a visualização do monitor serial estar aberta para poder prosseguir
    while (!stdio_usb_connected());

    // Cria a tarefa de inicialização dos componentes com prioridade 3
    xTaskCreate(tarefa_self_test, "Start", 512, NULL, 3, NULL);


    // Cria a tarefa de controle dos LEDs com prioridade 2
    xTaskCreate(tarefa_alive_task, "LEDs", 512, NULL, 2, NULL);

    // Cria a tarefa de leitura do joystick com prioridade 1
    // xTaskCreate(tarefa_monitor_joystick_alarme, "Joystick", 512, NULL, 1, NULL);

    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    while (true);

    return 0;
}