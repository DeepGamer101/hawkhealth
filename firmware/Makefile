# HawkHealth firmware -- STM32F767 (Renode + board), arm-none-eabi-gcc.
CC      := arm-none-eabi-gcc
SIZE    := arm-none-eabi-size
KERNEL  := third_party/FreeRTOS-Kernel
PORT    := $(KERNEL)/portable/GCC/ARM_CM7/r0p1

CPU     := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
CFLAGS  := $(CPU) -ffreestanding -O2 -Wall -Wextra -g3 \
           -ffunction-sections -fdata-sections \
           -Iinclude -I$(KERNEL)/include -I$(PORT)
LDFLAGS := $(CPU) -nostartfiles -T src/startup/stm32f767.ld -Wl,--gc-sections

ifeq ($(TEST),1)
CFLAGS += -DHH_BUILD_TEST
TARGET := hawkhealth_test.elf
else
TARGET := hawkhealth.elf
endif

APP_SRC := $(wildcard src/*/*.c)

KERNEL_SRC := $(KERNEL)/tasks.c \
              $(KERNEL)/list.c \
              $(KERNEL)/queue.c \
              $(PORT)/port.c \
              $(KERNEL)/portable/MemMang/heap_4.c

SRC := $(APP_SRC) $(KERNEL_SRC)

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $@
	$(SIZE) $@

clean:
	rm -f *.elf
