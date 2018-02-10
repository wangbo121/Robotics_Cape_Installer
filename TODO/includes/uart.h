/*******************************************************************************
* UART
*
* These C functions operate on the linux file system devices /dev/ttyO*
* but provide a tidy API for use in C functions
*******************************************************************************/
#ifndef RC_UART_H
#define RC_UART_H

int rc_uart_init(int bus, int speed, float timeout);
int rc_uart_close(int bus);
int rc_uart_fd(int bus);
int rc_uart_send_bytes(int bus, int bytes, char* data);
int rc_uart_send_byte(int bus, char data);
int rc_uart_read_bytes(int bus, int bytes, char* buf);
int rc_uart_read_line(int bus, int max_bytes, char* buf);
int rc_uart_flush(int bus);
int rc_uart_bytes_available(int bus);

#endif // RC_UART_H