/* hh_command.h -- S5 COMMAND interface. Parses inbound UART commands. Owner: (assigned). */
#ifndef HH_COMMAND_H
#define HH_COMMAND_H
#include "hh_types.h"
void HH_Command_Init(void);
void HH_Command_Feed(char c);   /* push one received byte (called from RX path/ISR) */
void HH_Command_Task(void *pv); /* parse buffered input -> CONFIG writes */
#endif
