/* hh_telemetry.c -- S4 TELEMETRY: format reading+alert; telemetryQ -> UART. */
#include "hh_telemetry.h"
#include "hh_platform.h"
#include "hh_system.h"
#include "hh_health.h"
#include "FreeRTOS.h"
#include "task.h"

static const char *alert_str(HH_AlertLevel_t a){
    return (a==HH_ALERT_CRITICAL) ? "CRITICAL" : (a==HH_ALERT_HIGH) ? "HIGH" : "NONE";
}

void HH_Telemetry_Init(void){ }

void HH_Telemetry_Send(const HH_TelemetryMsg_t *m){
    hh_print("[HH] t=");    hh_print_u32(m->reading.timestamp_ms);
    hh_print(" temp=");     hh_print_float1(m->reading.temp_c);
    hh_print(" spo2=");     hh_print_float1(m->reading.spo2_pct);
    hh_print(" hr=");       hh_print_u32(m->reading.heart_rate_bpm);
    hh_print(" alert=");    hh_println(alert_str(m->alert));
}

void HH_Telemetry_Task(void *pv){
    (void)pv;
    HH_TelemetryMsg_t m;
    for(;;){
        if(xQueueReceive(telemetryQ, &m, portMAX_DELAY) == pdTRUE){
            HH_Telemetry_Send(&m);
        }
        HH_Health_Heartbeat(HH_TASK_TELEMETRY);
    }
}
