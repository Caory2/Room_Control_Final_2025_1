#include "command_parser.h"
#include <string.h>
#include <stdio.h>



// Agrega esta línea:
extern room_control_t room_system;

#define CMD_BUFFER_SIZE 32
static char esp01_cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t esp01_cmd_index = 0;

static char debug_cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t debug_cmd_index = 0;

void command_parser_process_esp01(uint8_t rx_byte) {
    // Implementación para procesar comandos recibidos por ESP-01 (USART3)
        if (rx_byte == '\n' || rx_byte == '\r') {
        esp01_cmd_buffer[esp01_cmd_index] = '\0';
        command_parser_process(&room_system, esp01_cmd_buffer);
        esp01_cmd_index = 0;
    } else if (esp01_cmd_index < CMD_BUFFER_SIZE - 1) {
        esp01_cmd_buffer[esp01_cmd_index++] = rx_byte;
    }
}

void command_parser_process_debug(uint8_t rx_byte) {
    // Implementación para procesar comandos recibidos por debug (USART2)
    if (rx_byte == '\n' || rx_byte == '\r') {
        debug_cmd_buffer[debug_cmd_index] = '\0';
        command_parser_process(&room_system, debug_cmd_buffer);
        debug_cmd_index = 0;
    } else if (debug_cmd_index < CMD_BUFFER_SIZE - 1) {
        debug_cmd_buffer[debug_cmd_index++] = rx_byte;
    }

}

void command_parser_process(room_control_t *room, const char *cmd) {
    if (strncmp(cmd, "GET_TEMP", 8) == 0) {
        // Devolver temperatura actual
        float temp = room_control_get_temperature(room);
        printf("TEMP: %.1f C\r\n", temp);
    } else if (strncmp(cmd, "GET_STATUS", 10) == 0) {
        // Estado sistema y fan
        printf("STATE: %s\r\n", room_control_get_state(room) == ROOM_STATE_LOCKED ? "LOCKED" : "UNLOCKED");
        printf("FAN: %d\r\n", room_control_get_fan_level(room));
    } else if (strncmp(cmd, "SET_PASS:", 9) == 0) {
        // Cambiar contraseña
        const char *new_pass = cmd + 9;
        room_control_change_password(room, new_pass);
        printf("PASS CHANGED\r\n");
    } else if (strncmp(cmd, "FORCE_FAN:", 10) == 0) {
        // Forzar velocidad ventilador
        int level = cmd[10] - '0';
        if (level >= 0 && level <= 3) {
            room_control_force_fan_level(room, (fan_level_t)level);
            printf("FAN FORCED TO %d\r\n", level);
        } else {
            printf("INVALID FAN LEVEL\r\n");
        }
    } else {
        printf("UNKNOWN COMMAND\r\n");
    }
}