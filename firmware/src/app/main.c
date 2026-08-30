/* main.c -- HawkHealth entry. Default = full system pipeline; -DHH_BUILD_TEST = testbench. */
#include "FreeRTOS.h"
#include "task.h"
#if defined(HH_BUILD_TEST)
extern void hawkhealth_test_start(void);
#else
#include "hh_system.h"
#endif
int main(void){
#if defined(HH_BUILD_TEST)
    hawkhealth_test_start();
#else
    hh_app_start();
#endif
    vTaskStartScheduler();
    for(;;){}
    return 0;
}
