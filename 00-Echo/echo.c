#include <stdint.h>
#include "reg.h"

#define USART_FLAG_TXE  ((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x0020)

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

void echo ()
{
        while(1) {
                put_char(get_char());
        }
}

void main(void)
{
        *(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
        *(RCC_APB1ENR) |= (uint32_t) (0x00020000);

        /* USART2 Configuration */
        *(GPIOA_CRL) = 0x00004B00;
        *(GPIOA_CRH) = 0x44444444;

        *(USART2_CR1) = 0x0000000C;
        *(USART2_CR1) |= 0x2000;

        while (1) {
                echo();
        }
}
