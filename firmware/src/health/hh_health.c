/* hh_health.c -- S7 HEALTH: every task reports a heartbeat; this task watches them,
 * toggles the status LED as an "alive" indicator, and flags any silent task. */
#include "hh_health.h"
#include "hh_platform.h"
#include "FreeRTOS.h"
#include "task.h"

static volatile uint32_t s_last_beat[HH_TASK_COUNT];

void HH_Health_Init(void){
    for (int i = 0; i < HH_TASK_COUNT; i++) s_last_beat[i] = 0u;
}
void HH_Health_Heartbeat(HH_TaskId_t id){
    if (id < HH_TASK_COUNT) s_last_beat[id] = (uint32_t)xTaskGetTickCount();
}
void HH_Health_Task(void *pv){
    (void)pv;
    hh_println("[HH] health monitor up");
    const uint32_t timeout = (uint32_t)pdMS_TO_TICKS(2000);
    for (;;) {
        uint32_t now = (uint32_t)xTaskGetTickCount();
        for (int i = 0; i < HH_TASK_COUNT; i++) {
            if (s_last_beat[i] != 0u && (now - s_last_beat[i]) > timeout) {
                hh_print("[HH] health: task "); hh_print_u32((uint32_t)i); hh_println(" SILENT");
            }
        }
        hh_led_toggle();                 /* PB0 heartbeat = system alive */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
