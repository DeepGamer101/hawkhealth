/* hh_alert.c -- S3 ALERT: threshold check -> severity; procQ -> telemetryQ.
 * Reads live thresholds from CONFIG each evaluation (the shared-data path). */
#include "hh_alert.h"
#include "hh_config.h"      /* HH_DEFAULT_* thresholds (later: HH_Config_GetThresholds) */
#include "hh_system.h"
#include "hh_health.h"
#include "FreeRTOS.h"
#include "task.h"

void HH_Alert_Init(void){ }

HH_AlertLevel_t HH_Alert_Evaluate(const HH_SensorReading_t *r){
    float temp_high, spo2_crit;
    HH_Config_GetThresholds(&temp_high, &spo2_crit);   /* live values (shared state) */
    if(!r->valid) return HH_ALERT_NONE;
    if(r->spo2_pct < spo2_crit) return HH_ALERT_CRITICAL;   /* critical wins */
    if(r->temp_c  >= temp_high) return HH_ALERT_HIGH;
    return HH_ALERT_NONE;
}

void HH_Alert_Task(void *pv){
    (void)pv;
    HH_SensorReading_t r;
    HH_TelemetryMsg_t m;
    for(;;){
        if(xQueueReceive(procQ, &r, portMAX_DELAY) == pdTRUE){
            m.reading = r;
            m.alert   = HH_Alert_Evaluate(&r);
            (void)xQueueSend(telemetryQ, &m, portMAX_DELAY);
        }
        HH_Health_Heartbeat(HH_TASK_ALERT);
    }
}
