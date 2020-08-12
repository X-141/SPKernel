#include "Terminal.h"
#include "mini_uart.h"

unsigned int Pow(unsigned int aNum, unsigned int aPow) {
	unsigned int accumulator = aNum;
	for(unsigned int i = 1; i < aPow; i++)
		accumulator *= aNum;
	return accumulator;
}

void UintToString(unsigned int aNum, char* aEmptyString) {
	unsigned int loopNum = aNum;
	unsigned int numLength = 0;
	while((loopNum % Pow(10, numLength)) != aNum) 
		numLength++;
	
	for(int x = numLength-1; x >= 0 ; x--) {
		unsigned int division = (unsigned int)loopNum / 10;
		unsigned int remainder = loopNum % 10;
		aEmptyString[x] = (char)(remainder + 0x30);
		loopNum = division;
	}
	aEmptyString[numLength++] = '\n';
	aEmptyString[numLength++] = '\r';
	aEmptyString[numLength++] = '\0';
}

void init_input_buffer(struct input_buffer* buffer) {
    for(int x = 0; x < MAX_INPUT_BUFFER; x++)
        buffer->_buffer[x] = '\0';
    buffer->_buffer_size = 0;
}

void init_terminal() {
    struct input_buffer buff;
    init_input_buffer(&buff);

    enum terminal_status state = cNormal;
    // char debugvalue[10];
    // UintToString((int)'\n', debugvalue);
    // uart_send_string(debugvalue);
	// char debugvalue_1[10];
    // UintToString((int)'\r', debugvalue_1);
    // uart_send_string(debugvalue_1);

    uart_send_string("> ");
    while(state == cNormal) {
        state = _append_to_buffer(&buff, uart_recv());
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
    uart_send(value);
    // char debugvalue[10];
    // UintToString((int)value, debugvalue);
    // uart_send_string(debugvalue);
    return cNormal;
}

enum terminal_status 
_check_buffer(struct input_buffer* buffer) {
    // \n == 10
    // \r == 13
    // backspace is 127
    // When we hit the enter key three values are entered 13 13 10
    if(buffer->_buffer[buffer->_buffer_size-1] == '\n' ||
        buffer->_buffer[buffer->_buffer_size-1] == '\r') {
        _append_to_buffer(buffer, '\r');
        _append_to_buffer(buffer, '\n');
        _append_to_buffer(buffer, '\0');
        uart_send_string(buffer->_buffer);
        init_input_buffer(buffer);
        uart_send_string("> ");
    }
    return cNormal;
}