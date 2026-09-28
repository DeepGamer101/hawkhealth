/* hh_sensor.c -- deterministic sensor stub with injected faults + producer task.
 * call 300 -> temp_c 38.5 (HIGH); call 500 -> spo2 88.0 (CRITICAL). */
#include "hh_sensor.h"
#include "hh_system.h"
#include "hh_config.h"
#include "hh_health.h"
#include "FreeRTOS.h"
#include "task.h"

static bool     s_init = false;
static uint32_t s_calls = 0u;
uint32_t HH_Sensor_GetCallCount(void){ return s_calls; }

#define TEMP_BASE 36.8f
#define TEMP_DRIFT 0.001f
#define HUM_BASE 45.0f
#define HUM_DRIFT 0.003f
#define SPO2_BASE 98.0f
#define SPO2_DRIFT 0.001f
#define HR_BASE 72u
#define INJECT_HIGH_TEMP 300u
#define INJECT_CRIT_SPO2 500u

HH_SensorStatus_t HH_Sensor_Init(void){ s_init=true; s_calls=0u; return HH_SENSOR_OK; }

HH_SensorStatus_t HH_Sensor_Read(HH_SensorReading_t *out){
    if(out==0) return HH_SENSOR_ERR_BUS;
    if(!s_init){ out->timestamp_ms=0u; out->temp_c=0.0f; out->humidity_pct=0.0f; out->spo2_pct=0.0f; out->heart_rate_bpm=0u; out->valid=false; return HH_SENSOR_ERR_NOT_INIT; }
    s_calls++; const uint32_t n=s_calls;
    out->timestamp_ms   = n*HH_SAMPLE_PERIOD_MS;
    out->temp_c         = TEMP_BASE + (float)n*TEMP_DRIFT;
    out->humidity_pct   = HUM_BASE  + (float)n*HUM_DRIFT;
    out->spo2_pct       = SPO2_BASE - (float)n*SPO2_DRIFT;
    out->heart_rate_bpm = (uint16_t)(HR_BASE + (n%7u));
    out->valid          = true;
    if(n==INJECT_HIGH_TEMP) out->temp_c = 38.5f;
    if(n==INJECT_CRIT_SPO2) out->spo2_pct = 88.0f;
    return HH_SENSOR_OK;
}

void HH_Sensor_Task(void *pv){
    (void)pv;
    HH_SensorReading_t r;
    for(;;){
        if(HH_Config_IsRunning() && HH_Sensor_Read(&r) == HH_SENSOR_OK){
            (void)xQueueSend(rawQ, &r, portMAX_DELAY);
        }
        HH_Health_Heartbeat(HH_TASK_SENSOR);
        vTaskDelay(pdMS_TO_TICKS(10));   /* fast sim cadence; logical time via timestamp_ms */
    }
}
