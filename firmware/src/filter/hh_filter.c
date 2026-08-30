/* hh_filter.c -- S2 FILTER: validate/smooth raw readings; rawQ -> procQ. */
#include "hh_filter.h"
#include "hh_system.h"
#include "hh_health.h"
#include "FreeRTOS.h"
#include "task.h"

void HH_Filter_Init(void){ }

HH_SensorReading_t HH_Filter_Apply(const HH_SensorReading_t *raw){
    HH_SensorReading_t out = *raw;
    /* Reject grossly non-physiological values. (Keeps real faults like 38.5/88.0.) */
    if(out.temp_c < 20.0f || out.temp_c > 45.0f) out.valid = false;
    if(out.spo2_pct < 50.0f || out.spo2_pct > 100.0f) out.valid = false;
    return out;
}

void HH_Filter_Task(void *pv){
    (void)pv;
    HH_SensorReading_t r;
    for(;;){
        if(xQueueReceive(rawQ, &r, portMAX_DELAY) == pdTRUE){
            HH_SensorReading_t f = HH_Filter_Apply(&r);
            (void)xQueueSend(procQ, &f, portMAX_DELAY);
        }
        HH_Health_Heartbeat(HH_TASK_FILTER);
    }
}
