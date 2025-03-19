#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

// Definição dos pinos
const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

// Filas para comunicação entre tasks
QueueHandle_t xQueueButId_R; // Fila para BTN R e LED R
QueueHandle_t xQueueButId_G; // Fila para BTN G e LED G

// Task para controlar o LED R
void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0; // Valor inicial do delay
    while (true) {
        // Verifica se há um novo valor de delay na fila
        if (xQueueReceive(xQueueButId_R, &delay, 0)) {
            printf("LED R: Novo delay = %d\n", delay);
        }

        // Se o delay for maior que 0, pisca o LED R
        if (delay > 0) {
            gpio_put(LED_PIN_R, 1); // Liga o LED R
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
            gpio_put(LED_PIN_R, 0); // Desliga o LED R
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
        }
    }
}

// Task para controlar o LED G
void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0; // Valor inicial do delay
    while (true) {
        // Verifica se há um novo valor de delay na fila
        if (xQueueReceive(xQueueButId_G, &delay, 0)) {
            printf("LED G: Novo delay = %d\n", delay);
        }

        // Se o delay for maior que 0, pisca o LED G
        if (delay > 0) {
            gpio_put(LED_PIN_G, 1); // Liga o LED G
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
            gpio_put(LED_PIN_G, 0); // Desliga o LED G
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
        }
    }
}

// Task para ler o BTN R
void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    int delay = 0; // Valor inicial do delay
    while (true) {
        if (!gpio_get(BTN_PIN_R)) { // Verifica se o botão foi pressionado
            while (!gpio_get(BTN_PIN_R)) { // Aguarda o botão ser solto
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            // Incrementa o delay (ciclo de 100 a 1000 ms)
            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }

            printf("BTN R: Enviando delay = %d\n", delay);
            xQueueSend(xQueueButId_R, &delay, 0); // Envia o delay para a fila
        }
    }
}

// Task para ler o BTN G
void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    int delay = 0; // Valor inicial do delay
    while (true) {
        if (!gpio_get(BTN_PIN_G)) { // Verifica se o botão foi pressionado
            while (!gpio_get(BTN_PIN_G)) { // Aguarda o botão ser solto
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            // Incrementa o delay (ciclo de 100 a 1000 ms)
            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }

            printf("BTN G: Enviando delay = %d\n", delay);
            xQueueSend(xQueueButId_G, &delay, 0); // Envia o delay para a fila
        }
    }
}

int main() {
    stdio_init_all(); // Inicializa a stdio (para printf)
    printf("Start RTOS \n");

    // Cria as filas
    xQueueButId_R = xQueueCreate(32, sizeof(int)); // Fila para BTN R e LED R
    xQueueButId_G = xQueueCreate(32, sizeof(int)); // Fila para BTN G e LED G

    // Verifica se as filas foram criadas corretamente
    if (xQueueButId_R == NULL || xQueueButId_G == NULL) {
        printf("Erro ao criar filas!\n");
        while (true);
    }

    // Cria as tasks
    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL); // Task do LED R
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL); // Task do BTN R
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL); // Task do LED G
    xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL); // Task do BTN G

    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    // O programa nunca deve chegar aqui!
    while (true)
        ;
}