/* integration_test.c -- HawkHealth automated testbench.
 * Prints HAWKHEALTH_TEST_PASS or HAWKHEALTH_TEST_FAIL. CI greps for PASS. */
#if defined(HH_BUILD_TEST)
#include "FreeRTOS.h"
#include "task.h"
#include "hh_sensor.h"
#include "hh_platform.h"
#define TEST_SAMPLES 500u
#define HR_MIN 40u
#define HR_MAX 200u
static void report(const char *name,int ok){ hh_print("[HawkHealth] "); hh_print(name); hh_print(" ... "); hh_println(ok?"PASS":"FAIL"); }
static void vITestTask(void *pv){
    (void)pv; hh_platform_init(); hh_println("[HawkHealth] integration testbench starting");
    HH_SensorReading_t r, r1={0}, r300={0}, r500={0};
    int t1 = (HH_Sensor_Init()==HH_SENSOR_OK);
    int all_valid=1, ts_mono=1, hr_ok=1; uint32_t prev=0u;
    for(uint32_t n=1u;n<=TEST_SAMPLES;n++){
        HH_SensorStatus_t st=HH_Sensor_Read(&r);
        if(st!=HH_SENSOR_OK||!r.valid) all_valid=0;
        if(n>1u && r.timestamp_ms<=prev) ts_mono=0;
        prev=r.timestamp_ms;
        if(r.heart_rate_bpm<HR_MIN||r.heart_rate_bpm>HR_MAX) hr_ok=0;
        if(n==1u)r1=r; if(n==300u)r300=r; if(n==500u)r500=r;
    }
    int t2=(r1.valid && r1.temp_c>=30.0f && r1.temp_c<=42.0f);
    int t3=(r300.temp_c==38.5f)&&(r300.temp_c>=HH_TEMP_HIGH_C);
    int t4=(r500.spo2_pct==88.0f)&&(r500.spo2_pct<HH_SPO2_CRITICAL);
    int t5=all_valid&&ts_mono&&hr_ok;
    report("T1 sensor init",t1); report("T2 first reading valid",t2);
    report("T3 HIGH temp @ call300",t3); report("T4 CRIT spo2 @ call500",t4);
    report("T5 data integrity",t5);
    if(t1&&t2&&t3&&t4&&t5) hh_println("HAWKHEALTH_TEST_PASS"); else hh_println("HAWKHEALTH_TEST_FAIL");
    hh_halt();
}
void hawkhealth_test_start(void){ (void)xTaskCreate(vITestTask,"ITEST",configMINIMAL_STACK_SIZE*4,NULL,tskIDLE_PRIORITY+2,NULL); }
#endif
