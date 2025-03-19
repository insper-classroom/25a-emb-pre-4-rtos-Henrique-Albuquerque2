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
SemaphoreHandle_t xSemaphore_r;  
SemaphoreHandle_t xSemaphore_g;  

void btn_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio == BTN_PIN_R && events == 0x4) { // Fall edge no BTN R
        xSemaphoreGiveFromISR(xSemaphore_r, &xHigherPriorityTaskWoken);
    } else if (gpio == BTN_PIN_G && events == 0x4) { // Fall edge no BTN G
        xSemaphoreGiveFromISR(xSemaphore_g, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0; 
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) {
            printf("LED R: Novo delay = %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1); // Liga o LED R
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
            gpio_put(LED_PIN_R, 0); // Desliga o LED R
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0; // Valor inicial do delay
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) {
            printf("LED G: Novo delay = %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_G, 1); // Liga o LED G
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
            gpio_put(LED_PIN_G, 0); // Desliga o LED G
            vTaskDelay(pdMS_TO_TICKS(delay)); // Aguarda o tempo do delay
        }
    }
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    int delay = 0; // Valor inicial do delay
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }

            printf("BTN R: Enviando delay = %d\n", delay);
            xQueueSend(xQueueButId, &delay, 0); // Envia o delay para a fila
        }
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    int delay = 0; // Valor inicial do delay
    while (true) {
        // Aguarda o semáforo ser liberado pelo BTN G
        if (xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE) {
            // Incrementa o delay (ciclo de 100 a 1000 ms)
            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }

            printf("BTN G: Enviando delay = %d\n", delay);
            xQueueSend(xQueueButId, &delay, 0); // Envia o delay para a fila
        }
    }
}

int main() {
    stdio_init_all(); // Inicializa a stdio (para printf)
    printf("Start RTOS \n");

    // Cria a fila e os semáforos
    xQueueButId = xQueueCreate(32, sizeof(int)); // Fila para enviar o valor de delay
    xSemaphore_r = xSemaphoreCreateBinary();     // Semáforo para BTN R
    xSemaphore_g = xSemaphoreCreateBinary();     // Semáforo para BTN G

    // Verifica se a fila e os semáforos foram criados corretamente
    if (xQueueButId == NULL || xSemaphore_r == NULL || xSemaphore_g == NULL) {
        printf("Erro ao criar fila ou semáforos!\n");
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