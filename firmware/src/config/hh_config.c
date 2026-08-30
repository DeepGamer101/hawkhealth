/* hh_config.c -- S6 CONFIG: thresholds + run state, guarded by a mutex.
 * ALERT reads these; COMMAND writes them. The mutex is the shared-data lesson. */
#include "hh_config.h"
#include "FreeRTOS.h"
#include "semphr.h"

static float             s_temp_high;
static float             s_spo2_crit;
static bool              s_running;
static SemaphoreHandle_t s_mtx;

void HH_Config_Init(void){
    s_temp_high = HH_DEFAULT_TEMP_HIGH_C;
    s_spo2_crit = HH_DEFAULT_SPO2_CRIT_PCT;
    s_running   = true;
    s_mtx       = xSemaphoreCreateMutex();
}
void HH_Config_GetThresholds(float *temp_high_c, float *spo2_critical_pct){
    xSemaphoreTake(s_mtx, portMAX_DELAY);
    *temp_high_c = s_temp_high; *spo2_critical_pct = s_spo2_crit;
    xSemaphoreGive(s_mtx);
}
void HH_Config_SetTempHigh(float t){ xSemaphoreTake(s_mtx, portMAX_DELAY); s_temp_high = t; xSemaphoreGive(s_mtx); }
void HH_Config_SetSpo2Critical(float s){ xSemaphoreTake(s_mtx, portMAX_DELAY); s_spo2_crit = s; xSemaphoreGive(s_mtx); }
bool HH_Config_IsRunning(void){ bool r; xSemaphoreTake(s_mtx, portMAX_DELAY); r = s_running; xSemaphoreGive(s_mtx); return r; }
void HH_Config_SetRunning(bool running){ xSemaphoreTake(s_mtx, portMAX_DELAY); s_running = running; xSemaphoreGive(s_mtx); }
