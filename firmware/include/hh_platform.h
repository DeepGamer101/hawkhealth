/* hh_platform.h -- HawkHealth platform abstraction (STM32F767: Renode + board) */
#ifndef HH_PLATFORM_H
#define HH_PLATFORM_H
#include <stdint.h>
void hh_platform_init(void);          /* bring up USART3 + LED GPIO */
void hh_putc(char c);                 /* blocking UART write */
void hh_print(const char *s);
void hh_println(const char *s);
void hh_print_u32(uint32_t v);
void hh_print_float1(float f);        /* one decimal, e.g. 38.5 / 88.0 */
void hh_led_toggle(void);
void hh_led_set(int on);
int  hh_uart_rx_ready(void);   /* 1 if a byte is waiting */
char hh_uart_getc(void);       /* read one received byte */
void hh_halt(void);                   /* mask IRQs and spin (faults/testbench end) */
#endif
