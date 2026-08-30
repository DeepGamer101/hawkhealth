/* hh_telemetry.h -- S4 TELEMETRY interface. Formats + sends over UART. Owner: (assigned). */
#ifndef HH_TELEMETRY_H
#define HH_TELEMETRY_H
#include "hh_types.h"
void HH_Telemetry_Init(void);
void HH_Telemetry_Send(const HH_TelemetryMsg_t *m); /* format one message to UART */
void HH_Telemetry_Task(void *pv);                    /* telemetryQ -> UART */
#endif
