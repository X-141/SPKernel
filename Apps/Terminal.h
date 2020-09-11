#ifndef TERMINAL_H
#define TERMINAL_H

#define MAX_INPUT_BUFFER 512

struct input_buffer {
    char _buffer[MAX_INPUT_BUFFER];
    int _buffer_size;
};

void init_input_buffer(struct input_buffer* buffer);

enum terminal_status {
    cNormal,
    cExit
};

void init_terminal();

enum terminal_status 
_append_to_buffer(struct input_buffer* buffer, char value);

enum terminal_status 
_check_buffer(struct input_buffer* buffer);

enum terminal_status 
_remove_from_buffer(struct input_buffer* buffer);

#endif