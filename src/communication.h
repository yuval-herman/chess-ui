#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <stdbool.h>

bool pipe_init();
void pipe_close();
bool pipe_is_connected();
bool pipe_send_message(char* msg);
bool pipe_has_new_message();
char *pipe_get_message();

#endif // COMMUNICATION_H
