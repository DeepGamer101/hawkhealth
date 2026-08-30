/* hh_types.h -- shared HawkHealth types. The base every subsystem codes against. */
#ifndef HH_TYPES_H
#define HH_TYPES_H
#include <stdint.h>
#include <stdbool.h>

/* One set of vital-sign readings (raw from SENSOR, or validated by FILTER). */
typedef struct {
    uint32_t timestamp_ms;
    float    temp_c;
    float    humidity_pct;
    float    spo2_pct;
    uint16_t heart_rate_bpm;
    bool     valid;
} HH_SensorReading_t;

/* Alert severity assigned to a reading by ALERT. */
typedef enum {
    HH_ALERT_NONE = 0,
    HH_ALERT_HIGH,        /* a value crossed a HIGH threshold     */
    HH_ALERT_CRITICAL     /* a value crossed a CRITICAL threshold */
} HH_AlertLevel_t;

/* What flows to TELEMETRY: a reading plus its alert verdict. */
typedef struct {
    HH_SensorReading_t reading;
    HH_AlertLevel_t    alert;
} HH_TelemetryMsg_t;

/* Task identifiers for the HEALTH monitor's heartbeat table. */
typedef enum {
    HH_TASK_SENSOR = 0,
    HH_TASK_FILTER,
    HH_TASK_ALERT,
    HH_TASK_TELEMETRY,
    HH_TASK_COMMAND,
    HH_TASK_COUNT
} HH_TaskId_t;

#endif /* HH_TYPES_H */
