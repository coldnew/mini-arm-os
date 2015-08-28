CROSS_COMPILE ?= arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
CFLAGS = -fno-common -O0 \
	 -mcpu=cortex-m3 -mthumb \
	 -T echo.ld -nostartfiles \

TARGET = echo.bin
all: $(TARGET)

$(TARGET): echo.c startup.c
	$(CC) $(CFLAGS) $^ -o echo.elf
	$(CROSS_COMPILE)objcopy -Obinary echo.elf echo.bin
	$(CROSS_COMPILE)objdump -S echo.elf > echo.list

qemu: $(TARGET)
	@qemu-system-arm -M ? | grep stm32-p103 >/dev/null || exit
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo
	qemu-system-arm -M stm32-p103 -nographic -kernel echo.bin

clean:
	rm -f *.o *.bin *.elf *.list
