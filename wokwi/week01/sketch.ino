/* HawkHealth — Week 1 RTOS concept sketch (Wokwi, ST Nucleo-C031C6).
 *
 * Two independent FreeRTOS tasks on a real ST Nucleo. The point is to SEE concurrency:
 * two for(;;) loops that each act like they own the CPU, taking turns because each one
 * BLOCKS (vTaskDelay) instead of spinning.
 *
 * Matched to a known-working Wokwi C031C6 FreeRTOS project. Two Wokwi-specific rules:
 *   1. Include FreeRTOS.h / task.h directly (the STM32 Arduino core provides them).
 *   2. Do NOT call vTaskStartScheduler() — the Arduino core starts the scheduler for you
 *      after setup() returns. (On real bare-metal HawkHealth, main() DOES call it — that
 *      difference is the Arduino framework hiding a step, not a different RTOS.)
 *
 * REQUIRED LIBRARY: in Wokwi, open the Library Manager tab and add "STM32duino FreeRTOS"
 * (or rely on the included libraries.txt). Without it, FreeRTOS.h will not be found.
 *
 * Notice the task shape: xTaskCreate + vTaskDelay are the EXACT FreeRTOS calls HawkHealth
 * uses on the STM32F767. Only the I/O lines (digitalWrite / Serial) differ from
 * HawkHealth's hh_led_toggle() / hh_putc(). That difference is the "platform seam."
 */
#include <Arduino.h>      // Wokwi uses the Arduino framework for STM32
#include "FreeRTOS.h"
#include "task.h"

#define LED_A PA5          // on-board LED on the Nucleo-C031C6
#define LED_B PB1          // second LED you add in the Wokwi diagram

static void TaskA(void *pv) {          // fast blinker + heartbeat print
    (void)pv;
    for (;;) {
        digitalWrite(LED_A, !digitalRead(LED_A));
        Serial.println("A: tick (250ms)");
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void TaskB(void *pv) {          // slow blinker + PING print
    (void)pv;
    for (;;) {
        digitalWrite(LED_B, !digitalRead(LED_B));
        Serial.println("B: PING (750ms)");
        vTaskDelay(pdMS_TO_TICKS(750));
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_A, OUTPUT);
    pinMode(LED_B, OUTPUT);
    xTaskCreate(TaskA, "TaskA", 128, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(TaskB, "TaskB", 128, NULL, tskIDLE_PRIORITY + 1, NULL);
    // No vTaskStartScheduler() here — Wokwi's STM32 Arduino core starts it after setup().
}

void loop() {
    vTaskDelay(1);                      // yield to the scheduler
}
