#ifndef USART_UTILS_H
#define USART_UTILS_H

#include <stdint.h>

void USART_config(uint32_t baudrate);
void USART_Send(char c);
void USART_putString(char *string);

#endif