/* HawkHealth — Week 2a concept sketch (Wokwi, ST Nucleo-C031C6).
 *
 * Chapter 2 hands-on: GPIO INPUT. One task blinks an LED; a second task polls a push
 * button and prints on each fresh press. Two independent FreeRTOS tasks, plus your first
 * board input — the same input side HawkHealth's COMMAND subsystem needs later.
 *
 * REQUIRED LIBRARY: Wokwi Library Manager -> + Add -> "STM32duino FreeRTOS"  (provides <STM32FreeRTOS.h>).
 * DIAGRAM: add a pushbutton — one leg to PB2, the diagonal leg to GND (uses the internal pull-up).
 */
#include <STM32FreeRTOS.h>

#define LED_PIN LED_BUILTIN   /* on-board LED (PA5) */
#define BTN_PIN PB2           /* pushbutton to GND, using internal pull-up */

static void BlinkTask(void *pv) {
    (void)pv;
    pinMode(LED_PIN, OUTPUT);
    for (;;) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

static void ButtonTask(void *pv) {
    (void)pv;
    pinMode(BTN_PIN, INPUT_PULLUP);
    int last = HIGH;
    for (;;) {
        int now = digitalRead(BTN_PIN);
        if (now == LOW && last == HIGH) {       /* falling edge = a fresh press */
            Serial.println("BUTTON pressed");
        }
        last = now;
        vTaskDelay(pdMS_TO_TICKS(20));          /* 20 ms poll = simple debounce */
    }
}

void setup() {
    Serial.begin(115200);
    xTaskCreate(BlinkTask,  "Blink",  128, NULL, 1, NULL);
    xTaskCreate(ButtonTask, "Button", 128, NULL, 2, NULL);
    vTaskStartScheduler();              /* start the scheduler (required) */
}

void loop() { }
