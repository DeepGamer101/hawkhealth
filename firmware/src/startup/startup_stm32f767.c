/* startup_stm32f767.c -- vector table + C startup for STM32F767 (Cortex-M7). */
#include <stdint.h>
extern uint32_t _sidata,_sdata,_edata,_sbss,_ebss,_estack;
extern int main(void);
extern void SVC_Handler(void);
extern void PendSV_Handler(void);
extern void SysTick_Handler(void);
void Reset_Handler(void);
void Default_Handler(void);
void NMI_Handler(void)        __attribute__((weak,alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak,alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak,alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak,alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak,alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak,alias("Default_Handler")));
__attribute__((section(".isr_vector"),used))
void (* const g_vectors[])(void) = {
    (void(*)(void))(&_estack), Reset_Handler, NMI_Handler, HardFault_Handler,
    MemManage_Handler, BusFault_Handler, UsageFault_Handler, 0,0,0,0,
    SVC_Handler, DebugMon_Handler, 0, PendSV_Handler, SysTick_Handler,
    /* a pad of external IRQ slots (none enabled by this firmware) */
    Default_Handler,Default_Handler,Default_Handler,Default_Handler,
    Default_Handler,Default_Handler,Default_Handler,Default_Handler,
    Default_Handler,Default_Handler,Default_Handler,Default_Handler,
    Default_Handler,Default_Handler,Default_Handler,Default_Handler,
};
static void enable_fpu(void){ volatile uint32_t *cpacr=(volatile uint32_t*)0xE000ED88UL; *cpacr|=(0xFu<<20); __asm volatile("dsb"); __asm volatile("isb"); }
void Reset_Handler(void){
    enable_fpu();
    uint32_t *s=&_sidata,*d=&_sdata; while(d<&_edata) *d++=*s++;
    for(d=&_sbss; d<&_ebss;) *d++=0u;
    (void)main();
    for(;;){}
}
void Default_Handler(void){ for(;;){} }
