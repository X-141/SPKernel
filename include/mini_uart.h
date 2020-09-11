#ifndef	_MINI_UART_H
#define	_MINI_UART_H

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_send_string(char* str);

// Source raspi3-tutorial uart.c
void uart_hex(unsigned int d);

// Source raspi3-tutorial uart.c
void uart_dump(void *ptr);

#endif  /*_MINI_UART_H */
