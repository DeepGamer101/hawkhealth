/* hh_filter.h -- S2 FILTER interface. Validates/smooths raw readings. Owner: (assigned). */
#ifndef HH_FILTER_H
#define HH_FILTER_H
#include "hh_types.h"
void               HH_Filter_Init(void);
HH_SensorReading_t HH_Filter_Apply(const HH_SensorReading_t *raw); /* pure fn (testable) */
void               HH_Filter_Task(void *pv);      /* rawQ -> procQ */
#endif
