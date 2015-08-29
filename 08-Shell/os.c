#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"

/* use some toolchain implement function */
#include <string.h>
#include <stdlib.h>

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

#define CR  0x0d
#define BS  0x08
#define DEL 0x7f

/* Implement simple mutex for serial readl/write */
unsigned char __usart2_lock = 0;

void usart2_trylock()
{
	__usart2_lock = 1;
}

void usart2_lock()
{
	while(1 == __usart2_lock);	
}

void usart2_unlock()
{
	__usart2_lock = 0;
}

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

void put_char(const char c)
{
	while (!(*(USART2_SR) & USART_FLAG_TXE));
	*(USART2_DR) = c & 0xFF;
}

void print_str(const char *str)
{
	while (*str) {
		put_char(*str);
		str++;
	}
}

void print_str_lock(const char *str)
{
	usart2_lock();
	print_str(str);
}

unsigned char get_char()
{
	while (!(*(USART2_SR) & USART_FLAG_RXNE));
	return  (unsigned char) (*(USART2_DR) & 0xFF);;
}

int get_str(char *s)
{
	int cnt = 0;
	char c;

	while ((c = get_char()) != CR) {
		/* lock resource */
		usart2_trylock();
		/* skip Backspace & Delete key */
		if ((c != BS) && (c != DEL)) {
			cnt++;
			*s++ = c;
			put_char(c);
		}
		else {
			if (cnt > 0) {
				cnt--;
				*s-- = ' ';
				print_str("\b \b");
			}
		}
	}
	*s = 0;
	usart2_unlock();
	return cnt;
}

static int findGCD_v1(int a, int b) {
	print_str("running findGVD_v1\n");
        while (1) {
                if (a > b) a -= b;
                else if (a < b) b -= a;
                else return a;
        }
}

static int findGCD_v2(int a, int b) {
	print_str("running findGVD_v2\n");
        while (1) {
                a %= b;
                if (a == 0) return b;
                if (a == 1) return 1;
                b %= a;
                if (b == 0) return a;
                if (b == 1) return 1;
        }
}

static int fib(int n)
{
	if (0 == n) return 0;
	if (1 == n) return 1;
	return (fib(n - 1) + fib(n - 2));
}

void fib_thread(void *userdata)
{
	char buf[32];
	while(1) {
		for (int i = 2; i < 3; ++i) {
			itoa(fib(i), buf, 10);
			print_str_lock("\nfibonacci seq :");
			print_str_lock(buf);
			print_str_lock("\n");
		}
	}
}

void findgcd_thread(void *userdata)
{
	int (*findgcd)(int, int);

	if (!strncmp(userdata, "findGCDv1", 9))
		findgcd = &findGCD_v1;
	else if (!strncmp(userdata, "findGCDv2", 9))
		findgcd = &findGCD_v2;
	else {
		print_str("ERROR: wrong useage on findgcd_thread\n");
		return;
	}

	print_str("Start findgcd_thread....\n");
	for(int i = 2;i < 9999 + 1; i++){
                for(int j = i + 1 ;j < 9999 + 1; j++){
                        findgcd(i,j);
                }
        }
	print_str("End findgcd_thread....\n");
}

void shell_thread(void *userdata)
{
	char buf[64] = { '\0' };

	while(1) {
		print_str("mini-arm-os $ ");
		get_str(buf);
		print_str("\n");
		if (!strncmp(buf, "help", 4)) {
			print_str("Usage:\n"
				  "  help      -- this document\n"
				  "  clear     -- clear screen\n"
				  "  findGCDv1 -- findGCD v1 \n"
				  "  findGCDv2 -- findGCD v2 \n"
				);
		}
		else if (!strncmp(buf, "clear", 4)) {
			put_char(27);
			print_str("[2J");
			put_char(27);
			print_str("[H");
		}
		else if (!strncmp(buf, "findGCDv1", 9)) {
			if (thread_create(findgcd_thread, (void *) "findGCDv1") == -1)
				print_str("findGCDv1 creation failed\r\n");
		}
		else if (!strncmp(buf, "findGCDv2", 9)) {
			if (thread_create(findgcd_thread, (void *) "findGCDv2") == -1)
				print_str("findGCDv2 creation failed\r\n");
		}
		else {
			if (buf[0] != 0) {
				print_str("    ");
				print_str(buf);
				print_str(": command not found\n");
			}
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

	if (thread_create(fib_thread, (void *) "fib_thread") == -1)
		print_str("fib thread creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
