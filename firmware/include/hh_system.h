/* hh_system.h -- system wiring: the queues that connect the pipeline, + app entry. */
#ifndef HH_SYSTEM_H
#define HH_SYSTEM_H
#include "FreeRTOS.h"
#include "queue.h"
extern QueueHandle_t rawQ;        /* SENSOR -> FILTER    (HH_SensorReading_t) */
extern QueueHandle_t procQ;       /* FILTER -> ALERT     (HH_SensorReading_t) */
extern QueueHandle_t telemetryQ;  /* ALERT  -> TELEMETRY (HH_TelemetryMsg_t)  */
void hh_app_start(void);          /* create queues + tasks, then caller starts scheduler */
#endif
