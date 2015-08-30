#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"
#include "string.h"
#include "shell.h"

/* use some toolchain implement function */
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

static int findGCD_v3(int a, int b) {
        while (1) {
                if (a > (b * 4)) {
                        a %= b;
                        if (a == 0) return b;
                        if (a == 1) return 1;
                } else if (a >= b) {
                        a -= b;
                        if (a == 0) return b;
                        if (a == 1) return 1;
                }
                if (b > (a * 4)) {
                        b %= a;
                        if (b == 0) return a;
                        if (b == 1) return 1;
                } else if (b >= a) {
                        b -= a;
                        if (b == 0) return a;
                        if (b == 1) return 1;
                }
        }
}

static int fib(int n)
{
	if (0 == n) return 0;
	if (1 == n) return 1;
	return (fib(n - 1) + fib(n - 2));
}

void cmd_fib(void *userdata)
{
	char buf[40];
	print_str("fib(");
	print_str(userdata);
	print_str(") = ");
	itoa(fib(atoi(userdata)), buf, 10);
	print_str(buf);
	print_str("\n");
}

void cmd_gcd(void *userdata)
{
	// reuse shell parser since I am lazy :)
	// FIXME: this method is really dangerous...
	shell_arg_parser(userdata, strlen(userdata), shell_args);

	print_str("cd(");
	print_str(shell_args[1]);
	print_str(", ");
	print_str(shell_args[2]);
	print_str(") = ");

	char buf [20];
	int a = atoi(shell_args[1]);
	int b = atoi(shell_args[2]);
	print_str(itoa(findGCD_v3(a, b), buf, 10));
	print_str("\n");
}

void shell_thread(void *userdata)
{
	char buf[64] = { '\0' };

	while(1) {
		print_str("mini-arm-os $ ");
		get_str(buf);
		print_str("\n");
		int argc = shell_arg_parser(buf, strlen(buf), shell_args);

		if (0 == argc) continue;

		if (!strcmp(shell_args[0], "help")) {
			print_str("Usage:\n"
				  "  help          -- this document\n"
				  "  clear         -- clear screen\n"
				  "  echo <val>    -- echo val\n"
				  "  fib  <val>    -- calculate fib(val) \n"
				  "  gcd  <a> <b>  -- calculate gcd(a, b) \n"
				);
		}
		else if (!strcmp(shell_args[0], "clear")) {
			put_char(27);
			print_str("[2J");
			put_char(27);
			print_str("[H");
		}
		else if (!strcmp(shell_args[0], "echo")) {
			print_str(shell_args[1]);
			put_char('\n');
		}
		else if (!strcmp(shell_args[0], "fib")) {
			int val = atoi(shell_args[1]);
			if ((val < 0) || (val > 47)) {
				print_str("ERROR: val should in 0 ~ 47\n");
				continue;
			}
			// detect if we need run fib in thread
			if (!strcmp(shell_args[2], "&")) {
				print_str("try to start fib thread....\n");
				if (thread_create(cmd_fib, (void *) shell_args[1]) == -1)
					print_str("shell thread creation failed\r\n");				
			}
			else {
				cmd_fib(shell_args[1]);
			}
		}
		else if (!strcmp(shell_args[0], "gcd")) {
			// detect if we need run fib in thread
			if (!strcmp(shell_args[3], "&")) {
				print_str("try to start gcd thread....\n");
				if (thread_create(cmd_gcd, (void *) buf) == -1)
					print_str("shell thread creation failed\r\n");				
			}
			else {
				cmd_gcd(buf);
			}
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

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
