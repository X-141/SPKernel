#include "Terminal.h"
#include "mini_uart.h"

#include "string.h"

void init_input_buffer(struct input_buffer* buffer) {
    for(int x = 0; x < MAX_INPUT_BUFFER; x++)
        buffer->_buffer[x] = '\0';
    buffer->_buffer_size = 0;
}

void init_terminal() {
    struct input_buffer buff;
    init_input_buffer(&buff);

    enum terminal_status state = cNormal;

    uart_send_string("> ");
    while(state == cNormal) {
        char recv_val = uart_recv();
        state = _append_to_buffer(&buff, recv_val);
        state = _check_buffer(&buff);
    }
    init_input_buffer(&buff);
}

enum terminal_status 
_append_to_buffer(struct input_buffer* buffer, char value) {
    // We need enough space to include \r\n\0
    if(buffer->_buffer_size + 3 == MAX_INPUT_BUFFER)
        init_input_buffer(buffer);
    buffer->_buffer[buffer->_buffer_size++] = value;
    // echo value back.
    uart_send(value);
    return cNormal;
}

enum terminal_status 
_check_buffer(struct input_buffer* buffer) {
    // \n == 10
    // \r == 13
    // backspace is 127
    // When we hit the enter key three values are entered 13 13 10
    if(buffer->_buffer[buffer->_buffer_size-1] == '\r') {
        _append_to_buffer(buffer, '\n');
        _append_to_buffer(buffer, '\0');
        uart_send_string(buffer->_buffer);
        init_input_buffer(buffer);
        uart_send_string("> ");
    }
    return cNormal;
}