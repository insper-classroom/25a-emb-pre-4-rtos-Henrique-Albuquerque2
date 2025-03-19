#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButId;
QueueHandle_t xQueueButId_G;

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) {
            printf("%d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    int delay = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_R)) {

            while (!gpio_get(BTN_PIN_R)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }
            printf("delay btn %d \n", delay);
            xQueueSend(xQueueButId, &delay, 0);
        }
    }
}

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
    stdio_init_all();
    printf("Start RTOS \n");

    xQueueButId = xQueueCreate(32, sizeof(int));

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
