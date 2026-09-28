/* hh_config.h -- S6 CONFIG interface. Owns thresholds + system state (mutex-guarded).
 * Owner: (assigned). */
#ifndef HH_CONFIG_H
#define HH_CONFIG_H
#include "hh_types.h"
#define HH_DEFAULT_TEMP_HIGH_C   38.0f
#define HH_DEFAULT_SPO2_CRIT_PCT 90.0f
void  HH_Config_Init(void);
void  HH_Config_GetThresholds(float *temp_high_c, float *spo2_critical_pct);
void  HH_Config_SetTempHigh(float temp_high_c);
void  HH_Config_SetSpo2Critical(float spo2_critical_pct);
bool  HH_Config_IsRunning(void);
void  HH_Config_SetRunning(bool running);
#endif
