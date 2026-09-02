/* hh_command.c -- S5 COMMAND: buffer inbound bytes into lines, parse, apply to CONFIG.
 * Commands (one per line):  TEMP <val> | SPO2 <val> | START | STOP
 * In Chunk 3 the task polls the UART RX; on hardware (Week 9) an RX ISR calls
 * HH_Command_Feed() instead -- same parser, different delivery. */
#include "hh_command.h"
#include "hh_config.h"
#include "hh_health.h"
#include "hh_platform.h"
#include "FreeRTOS.h"
#include "task.h"

static char s_buf[32];
static int  s_len;

static int prefix(const char *s, const char *p){ while(*p){ if(*s++ != *p++) return 0; } return 1; }
static float parse_float(const char *s){
    while(*s==' ') s++;
    int neg=0; if(*s=='-'){ neg=1; s++; }
    float v=0.0f;
    while(*s>='0'&&*s<='9'){ v = v*10.0f + (float)(*s-'0'); s++; }
    if(*s=='.'){ s++; float f=0.1f; while(*s>='0'&&*s<='9'){ v += (float)(*s-'0')*f; f*=0.1f; s++; } }
    return neg ? -v : v;
}
static void apply(const char *line){
    if      (prefix(line,"TEMP ")){ float v=parse_float(line+5); HH_Config_SetTempHigh(v);
                                     hh_print("[HH] cmd TEMP now "); hh_print_float1(v); hh_putc('\r'); hh_putc('\n'); }
    else if (prefix(line,"SPO2 ")){ float v=parse_float(line+5); HH_Config_SetSpo2Critical(v);
                                     hh_print("[HH] cmd SPO2 now "); hh_print_float1(v); hh_putc('\r'); hh_putc('\n'); }
    else if (prefix(line,"START")){ HH_Config_SetRunning(true);  hh_println("[HH] cmd START"); }
    else if (prefix(line,"STOP")) { HH_Config_SetRunning(false); hh_println("[HH] cmd STOP"); }
    else if (line[0] != 0)        { hh_print("[HH] cmd ? "); hh_println(line); }
}
void HH_Command_Init(void){ s_len = 0; }
void HH_Command_Feed(char c){
    if (c=='\r' || c=='\n'){ s_buf[s_len]=0; if(s_len>0) apply(s_buf); s_len=0; }
    else if (s_len < (int)sizeof(s_buf)-1){ s_buf[s_len++]=c; }
}
void HH_Command_Task(void *pv) {
	(void)pv;
	for (;;) {
		while (hh_uart_rx_ready()) HH_Command_Feed(hh_uart_getc());
		HH_Health_Heartbeat(HH_TASK_COMMAND);
		vTaskDelay(pdMS_TO_TICKS(20));   // <-- restore this: yield so lower tasks run
	}
}
