/* hh_sensor.h -- S1 SENSOR interface. Owner: (assigned). */
#ifndef HH_SENSOR_H
#define HH_SENSOR_H
#include "hh_types.h"
typedef enum { HH_SENSOR_OK=0, HH_SENSOR_ERR_NOT_INIT=1, HH_SENSOR_ERR_BUS=2 } HH_SensorStatus_t;
/* Reference thresholds (the live ones come from CONFIG at runtime). */
#define HH_TEMP_HIGH_C      38.0f
#define HH_SPO2_CRITICAL    90.0f
#define HH_SAMPLE_PERIOD_MS 1000u
HH_SensorStatus_t HH_Sensor_Init(void);
HH_SensorStatus_t HH_Sensor_Read(HH_SensorReading_t *out);
uint32_t          HH_Sensor_GetCallCount(void);
void              HH_Sensor_Task(void *pv);      /* produces onto rawQ */
#endif
