/* hh_platform.c -- STM32F767 USART3 + PB0 LED, register level.
 * Same code runs in Renode (STM32F767 model) and on the NUCLEO-F767ZI board. */
#include "hh_platform.h"
#define REG(a)       (*(volatile uint32_t *)(a))
#define RCC_AHB1ENR  REG(0x40023830u)
#define RCC_APB1ENR  REG(0x40023840u)
#define GPIOB_MODER  REG(0x40020400u)
#define GPIOB_ODR    REG(0x40020414u)
#define USART3_CR1   REG(0x40004800u)
#define USART3_BRR   REG(0x4000480Cu)
#define USART3_ISR   REG(0x4000481Cu)
#define USART3_RDR   REG(0x40004824u)
#define USART3_TDR   REG(0x40004828u)
#define LED_PIN      0u
void hh_platform_init(void){
    RCC_AHB1ENR |= (1u<<1);
    RCC_APB1ENR |= (1u<<18);
    GPIOB_MODER  = (GPIOB_MODER & ~(3u<<(LED_PIN*2))) | (1u<<(LED_PIN*2));
    USART3_BRR   = 0x8Bu;
    USART3_CR1   = (1u<<3)|(1u<<2)|(1u<<0);   /* TE | RE | UE */
}
void hh_putc(char c){ while(!(USART3_ISR&(1u<<7))){} USART3_TDR=(uint32_t)(uint8_t)c; }
void hh_led_toggle(void){ GPIOB_ODR ^= (1u<<LED_PIN); }
void hh_led_set(int on){ if(on) GPIOB_ODR|=(1u<<LED_PIN); else GPIOB_ODR&=~(1u<<LED_PIN); }
void hh_print(const char *s){ while(*s) hh_putc(*s++); }
void hh_println(const char *s){ hh_print(s); hh_putc('\r'); hh_putc('\n'); }
void hh_print_u32(uint32_t v){ char b[11]; int i=0; if(v==0u){hh_putc('0');return;} while(v>0u){b[i++]=(char)('0'+(v%10u));v/=10u;} while(i-->0) hh_putc(b[i]); }
void hh_print_float1(float f){ if(f<0.0f){hh_putc('-');f=-f;} uint32_t w=(uint32_t)f; uint32_t t=(uint32_t)((f-(float)w)*10.0f+0.5f); if(t>=10u){w+=1u;t=0u;} hh_print_u32(w); hh_putc('.'); hh_putc((char)('0'+t)); }
int  hh_uart_rx_ready(void){ return (USART3_ISR & (1u<<5)) != 0; }   /* RXNE */
char hh_uart_getc(void){ return (char)(USART3_RDR & 0xFFu); }

void hh_halt(void){ __asm volatile("cpsid i"); for(;;) __asm volatile("wfi"); }
