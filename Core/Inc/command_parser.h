#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H


#include "room_control.h"

void command_parser_process(room_control_t *room, const char *cmd);
void command_parser_process_esp01(uint8_t rx_byte);
void command_parser_process_debug(uint8_t rx_byte);

#endif // COMMAND_PARSER_H