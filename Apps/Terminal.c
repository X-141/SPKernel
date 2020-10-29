#include "Terminal.h"
#include "mini_uart.h"

#include "string.h"

//! include dummy app
#include "ls.h"

// \n == 10
// \r == 13
// backspace is 127
// When we hit the enter key three values are entered 13 13 10

void _init_input_buffer(struct input_buffer* buffer) {
    for(int x = 0; x < MAX_INPUT_BUFFER; x++)
        buffer->_buffer[x] = '\0';
    buffer->_buffer_size = 0;
}

enum terminal_status 
_append_to_buffer(struct input_buffer* buffer, char value) {
    // We need enough space to include \r\n\0
    if(buffer->_buffer_size + 3 == MAX_INPUT_BUFFER)
        _init_input_buffer(buffer);
    buffer->_buffer[buffer->_buffer_size++] = value;
    // echo value back.
    uart_send(value);
    return cNormal;
}

enum terminal_status 
_check_buffer(struct input_buffer* buffer) {
    if(buffer->_buffer[buffer->_buffer_size-1] == '\r') {
        //! Here the user selected the enter key...
        //! That means we need to check the buffer if it matches the
        //! ls command.
        
        //! Echo out string
        _append_to_buffer(buffer, '\n');
        _append_to_buffer(buffer, '\0');
        uart_send_string(buffer->_buffer);

        //! Place terminating character at the position where starting
        //! '\r' was located. This is a shorthand way to allow for strcmp
        //! to not compare special characters.
        buffer->_buffer[buffer->_buffer_size-3] = '\0';
        //! Before we actually go about doing performing the test, I want
        //! to see if it actually works as intended. First lets grab the value
        //! returned from strcmp.
        //unsigned int compare_value = strcmp(buffer->_buffer, "ls");
        //if (compare_value == 0)
        //{
        //    list_directory();
        //}
        
        //! char str_compare_value[10];
        //! uint_to_string(str_compare_value, compare_value, 10);
        //! uart_send_string(str_compare_value);

        _init_input_buffer(buffer);
        uart_send_string("> ");
    }
    return cNormal;
}

enum terminal_status
_remove_from_buffer(struct input_buffer* buffer) {
    if(buffer->_buffer_size != 0)
        buffer->_buffer[--buffer->_buffer_size] = '\0';
        
    return cNormal;
}

void init_terminal() {
    uart_send_string("Initializing Terminal\r\n");

    struct input_buffer buff;
    _init_input_buffer(&buff);

    enum terminal_status state = cNormal;

    uart_send_string("Welcome to SPKernel!\r\n");
    uart_send_string("> ");
    while(state == cNormal) {
        char recv_val = uart_recv();
        
        // Any 0's passed will be ignored
        if(recv_val == 0)
            continue;

        // We need to go ahead and remove a character if we can
        if(recv_val == 127 && buff._buffer_size != 0) {
            // We replace entire line including the term prompt
            uart_send('\r'); 
            for(int x = 0; x <= buff._buffer_size+2; x++)
                uart_send(' ');
            uart_send('\r');
            _remove_from_buffer(&buff);
            // start from beginning, reprint line
            uart_send_string("> ");
            uart_send_string(buff._buffer);
            continue;
        }

        state = _append_to_buffer(&buff, recv_val);
        state = _check_buffer(&buff);
    }
}

