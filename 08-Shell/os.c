#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)

/* USART RXNE Flag
 * This flag is cleared by a read to the USARTx_DR register and
 * set when received data is ready to be read.
 * This flag can also be cleared by writing a zero to it.
 */
#define USART_FLAG_RXNE ((uint16_t) 0x0020)

#define CR 0x0d
#define BS 0x08

void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t) (0x00020000);

	/* USART2 Configuration, Rx->PA3, Tx->PA2 */
	*(GPIOA_CRL) = 0x00004B00;
	*(GPIOA_CRH) = 0x44444444;
	*(GPIOA_ODR) = 0x00000000;
	*(GPIOA_BSRR) = 0x00000000;
	*(GPIOA_BRR) = 0x00000000;

	*(USART2_CR1) = 0x0000000C;
	*(USART2_CR2) = 0x00000000;
	*(USART2_CR3) = 0x00000000;
	*(USART2_CR1) |= 0x2000;
}

void print_str(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

void put_char(const char c)
{
        while (!(*(USART2_SR) & USART_FLAG_TXE));
        *(USART2_DR) = c & 0xFF;
}

unsigned char get_char()
{
        while (!(*(USART2_SR) & USART_FLAG_RXNE));
        return (unsigned char) (*(USART2_DR) & 0xFF);
}

int get_str(char *s)
{
	int cnt = 0;
	char c;

	while ((c = get_char()) != CR) {
		if (c != BS) {
			cnt++;
			*s++ = c;
			put_char(c);
		}
		else {
			if (cnt > 0) {
				cnt--;
				*s-- = ' ';
				put_char('\b');
				put_char(' ');
				put_char('\b');
			}
		}
	}
	*s = 0;
	return cnt;
}

int strcmp(char *s1, char *s2)
{
	while (*s1 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}
	return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

static int findGCD_v1(int a, int b) {
        while (1) {
                if (a > b) a -= b;
                else if (a < b) b -= a;
                else return a;
        }
}

void findgcd_thread(void *userdata)
{
	int (*findgcd)(int, int);

	if (!strcmp(userdata, "findGCDv1"))
		findgcd = &findGCD_v1;
	else {
		print_str("ERROR: wrong useage on findgcd_thread\n");
		return;
	}

	for(int i = 2;i < 9999 + 1; i++){
                for(int j = i + 1 ;j < 9999 + 1; j++){
                        findgcd(i,j);
                }
		print_str("calculate findgcd\n");
        }
}

void shell_thread(void *userdata)
{
	char buf[64] = { '\0' };

	while(1) {
		print_str("\n mini-arm-os $ ");
		get_str(buf);
		print_str("\n");
		if (!strcmp(buf, "help")) {
			print_str("Usage:\n"
				  "  help      -- this document\n"
				  "  clear     -- clear screen\n"
				  "  findGCDv1 -- findGCD v1 \n"
				);
		}
		else if (!strcmp(buf, "clear")) {
			put_char(27);
			print_str("[2J");
			put_char(27);
			print_str("[H");
		}
		else if (!strcmp(buf, "findGCDv1")) {
			if (thread_create(findgcd_thread, (void *) "findGCDv1") == -1)
				print_str("findGCDv1 creation failed\r\n");
		}
	}
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
	usart_init();

	if (thread_create(shell_thread, (void *) "shell_thread") == -1)
		print_str("shell thread creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
