/* HawkHealth — Week 1 RTOS concept sketch (Wokwi, ST Nucleo-C031C6).
 *
 * Two independent FreeRTOS tasks on a real ST Nucleo. The point of this sketch is
 * to SEE concurrency: two for(;;) loops that each act like they own the CPU, taking
 * turns because each one BLOCKS (vTaskDelay) instead of spinning.
 *
 * Notice the shape: xTaskCreate + vTaskDelay are the EXACT FreeRTOS calls HawkHealth
 * uses on the STM32F767. Only the I/O lines (digitalWrite / Serial) differ from
 * HawkHealth's hh_led_toggle() / hh_putc(). That difference is the "platform seam."
 *
 * FIRST-RUN NOTE: this is a starter.  Wokwi uses the Arduino framework for STM32.
 */
#include <STM32FreeRTOS.h> /*Make sure that STM32duino FreeRTOS has been added in Library Manager*/

#define LED_A LED_BUILTIN   /* on-board LED (PA5 on the Nucleo-C031C6) */
#define LED_B PB1           /* second LED you add in the Wokwi diagram */

static void TaskA(void *pv) {           /* fast blinker + heartbeat print */
    (void)pv;
    pinMode(LED_A, OUTPUT);
    for (;;) {
        digitalWrite(LED_A, !digitalRead(LED_A));
        Serial.println("A: tick (250ms)");
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void TaskB(void *pv) {           /* slow blinker + PING print */
    (void)pv;
    pinMode(LED_B, OUTPUT);
    for (;;) {
        digitalWrite(LED_B, !digitalRead(LED_B));
        Serial.println("B: PING (750ms)");
        vTaskDelay(pdMS_TO_TICKS(750));
    }
}

void setup() {
    Serial.begin(115200);
    xTaskCreate(TaskA, "TaskA", 128, NULL, 1, NULL);
    xTaskCreate(TaskB, "TaskB", 128, NULL, 1, NULL);
    vTaskStartScheduler();              /* never returns */
}

void loop() { }                         /* unused: the scheduler runs everything */

