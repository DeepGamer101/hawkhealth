#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#define configUSE_PREEMPTION            1
#define configUSE_TIME_SLICING          1
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configUSE_16_BIT_TICKS          0
#define configCPU_CLOCK_HZ              ( 16000000UL )   /* HSI reset clock */
#define configTICK_RATE_HZ             ( 1000 )
#define configMAX_PRIORITIES           ( 5 )
#define configMINIMAL_STACK_SIZE       ( 128 )
#define configTOTAL_HEAP_SIZE          ( 64 * 1024 )
#define configMAX_TASK_NAME_LEN        ( 12 )
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configSUPPORT_STATIC_ALLOCATION  0
#define configCHECK_FOR_STACK_OVERFLOW   2
#define configUSE_MALLOC_FAILED_HOOK     1
#define configUSE_TIMERS               0
#define configUSE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES  1
#define configUSE_TASK_NOTIFICATIONS   1
#define configQUEUE_REGISTRY_SIZE      4
#define INCLUDE_vTaskDelay             1
#define INCLUDE_vTaskDelayUntil        1
#define INCLUDE_vTaskPrioritySet       1
#define INCLUDE_uxTaskPriorityGet      1
#define INCLUDE_vTaskDelete            1
#define INCLUDE_xTaskGetSchedulerState 1
/* Cortex-M7 (STM32F7 implements 4 priority bits) */
#define configPRIO_BITS                4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      ( (1u<<configPRIO_BITS)-1u )
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY       ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8-configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY  ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8-configPRIO_BITS) )
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
#define configASSERT( x ) if( (x)==0 ){ taskDISABLE_INTERRUPTS(); for(;;); }
#endif
