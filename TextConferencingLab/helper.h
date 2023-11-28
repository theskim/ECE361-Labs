#include "helper.c"

void clear_buffer(char* buf);
void smart_free(void** ptr);
void print_message(Message message);
void get_string_from_message(char* message_string, Message message);
void get_message_from_string(char* string_received, Message* message);