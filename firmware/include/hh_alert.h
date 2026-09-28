/* hh_alert.h -- S3 ALERT interface. Applies thresholds, raises alerts. Owner: (assigned). */
#ifndef HH_ALERT_H
#define HH_ALERT_H
#include "hh_types.h"
void            HH_Alert_Init(void);
HH_AlertLevel_t HH_Alert_Evaluate(const HH_SensorReading_t *r); /* pure fn (testable) */
void            HH_Alert_Task(void *pv);          /* procQ -> telemetryQ */
#endif
