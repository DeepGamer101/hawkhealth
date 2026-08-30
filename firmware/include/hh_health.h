/* hh_health.h -- S7 HEALTH interface. Heartbeat monitor + watchdog + status LED.
 * Owner: (assigned). */
#ifndef HH_HEALTH_H
#define HH_HEALTH_H
#include "hh_types.h"
void HH_Health_Init(void);
void HH_Health_Heartbeat(HH_TaskId_t id);  /* each task calls this every loop */
void HH_Health_Task(void *pv);             /* checks heartbeats, drives status LED */
#endif
