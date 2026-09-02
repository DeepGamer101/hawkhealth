/* hh_app.c -- HawkHealth system: queues + all seven subsystems.
 * SENSOR -> rawQ -> FILTER -> procQ -> ALERT -> telemetryQ -> TELEMETRY -> UART.
 * CONFIG is shared state (mutex); COMMAND writes it from UART; HEALTH watches heartbeats. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hh_system.h"
#include "hh_platform.h"
#include "hh_sensor.h"
#include "hh_filter.h"
#include "hh_alert.h"
#include "hh_telemetry.h"
#include "hh_command.h"
#include "hh_config.h"
#include "hh_health.h"

QueueHandle_t rawQ;
QueueHandle_t procQ;
QueueHandle_t telemetryQ;

void hh_app_start(void){
    hh_platform_init();
    hh_println("[HawkHealth] system starting");

    HH_Config_Init();          /* first: others read it */
    HH_Sensor_Init();
    HH_Filter_Init();
    HH_Alert_Init();
    HH_Telemetry_Init();
    HH_Command_Init();
    HH_Health_Init();

    rawQ       = xQueueCreate(8, sizeof(HH_SensorReading_t));
    procQ      = xQueueCreate(8, sizeof(HH_SensorReading_t));
    telemetryQ = xQueueCreate(8, sizeof(HH_TelemetryMsg_t));

    xTaskCreate(HH_Sensor_Task,    "SENSOR", configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+2, NULL);
    xTaskCreate(HH_Filter_Task,    "FILTER", configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+2, NULL);
    xTaskCreate(HH_Alert_Task,     "ALERT",  configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+2, NULL);
    xTaskCreate(HH_Telemetry_Task, "TELEM",  configMINIMAL_STACK_SIZE+64,  NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(HH_Command_Task,   "CMD",    configMINIMAL_STACK_SIZE+64,  NULL, tskIDLE_PRIORITY+3, NULL);
    xTaskCreate(HH_Health_Task,    "HEALTH", configMINIMAL_STACK_SIZE,     NULL, tskIDLE_PRIORITY+1, NULL);
}
