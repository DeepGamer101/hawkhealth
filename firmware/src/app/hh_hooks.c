/* hh_hooks.c -- FreeRTOS hooks, defined once for every build variant. */
#include "FreeRTOS.h"
#include "task.h"
#include "hh_platform.h"
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcName){
    (void)xTask; hh_print("[FATAL] stack overflow: "); hh_println(pcName?pcName:"?"); hh_halt();
}
void vApplicationMallocFailedHook(void){
    hh_println("[FATAL] malloc failed (heap too small)"); hh_halt();
}
