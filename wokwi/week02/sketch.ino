/* HawkHealth — Week 2 concept sketch (Wokwi, ST Nucleo-C031C6).
 *
 * Chapter 2 hands-on: GPIO INPUT. One task blinks an LED; a second task polls a push
 * button and prints on each fresh press. Two independent FreeRTOS tasks, plus your first
 * board input — the same input side HawkHealth's COMMAND subsystem needs later.
 *
 * REQUIRED LIBRARY: Wokwi Library Manager -> + Add -> "STM32duino FreeRTOS"
 * (or rely on libraries.txt). Without it, FreeRTOS.h will not be found.
 *
 * DIAGRAM: add a pushbutton — one leg to PB2, the diagonal leg to GND. The sketch uses the
 * internal pull-up, so PB2 reads HIGH until the button pulls it LOW.
 *
 * On real hardware you'd wire the button to an EXTI interrupt through the NVIC instead of
 * polling (that's Week 9). Polling here keeps it simple and reliable in the simulator.
 */
#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"

#define LED_PIN PA5      // on-board LED
#define BTN_PIN PB2      // pushbutton to GND, using internal pull-up

static void BlinkTask(void *pv) {
    (void)pv;
    for (;;) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

static void ButtonTask(void *pv) {
    (void)pv;
    int last = HIGH;
    for (;;) {
        int now = digitalRead(BTN_PIN);
        if (now == LOW && last == HIGH) {       // falling edge = a fresh press
            Serial.println("BUTTON pressed");
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(20));          // 20 ms poll = simple debounce
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    xTaskCreate(BlinkTask,  "Blink",  128, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ButtonTask, "Button", 128, NULL, tskIDLE_PRIORITY + 2, NULL);
    // No vTaskStartScheduler() — Wokwi's STM32 Arduino core starts it after setup().
}

void loop() { vTaskDelay(1); }
