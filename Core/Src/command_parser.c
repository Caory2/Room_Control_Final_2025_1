#include "command_parser.h"
#include <string.h>
#include <stdio.h>
#include "main.h"
#include <stdlib.h>
#include "temperature_sensor.h"

extern UART_HandleTypeDef huart3;



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
    char tx_buffer[64];
    char clean_cmd[CMD_BUFFER_SIZE];
    strncpy(clean_cmd, cmd, CMD_BUFFER_SIZE - 1);
    clean_cmd[CMD_BUFFER_SIZE - 1] = '\0';

    // Elimina espacios y saltos de línea al final
    int len = strlen(clean_cmd);
    while (len > 0 && (clean_cmd[len-1] == '\r' || clean_cmd[len-1] == '\n' || clean_cmd[len-1] == ' ')) {
        clean_cmd[--len] = '\0';
    }

    // DEBUG: imprime el comando recibido
    snprintf(tx_buffer, sizeof(tx_buffer), "CMD:[%s]\r\n", clean_cmd);
    HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 1000);

    if (strcmp(clean_cmd, "GET_TEMP") == 0) {
        int temp = (int)(temperature_sensor_read() + 0.5f); // Redondea al entero más cercano
        snprintf(tx_buffer, sizeof(tx_buffer), "TEMP: %d C\r\n", temp);
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 1000);
        
    } else if (strcmp(clean_cmd, "GET_STATUS") == 0) {
        snprintf(tx_buffer, sizeof(tx_buffer), "STATE: %s\r\n", room_control_get_state(room) == ROOM_STATE_LOCKED ? "LOCKED" : "UNLOCKED");
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 1000);
        snprintf(tx_buffer, sizeof(tx_buffer), "FAN: %d\r\n", room_control_get_fan_level(room));
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 1000);
    } else if (strncmp(clean_cmd, "SET_PASS:", 9) == 0) {
        // Copia y limpia el argumento
        char new_pass[8];
        strncpy(new_pass, clean_cmd + 9, sizeof(new_pass) - 1);
        new_pass[sizeof(new_pass) - 1] = '\0';
        // Elimina espacios y saltos de línea al final
        int plen = strlen(new_pass);
        while (plen > 0 && (new_pass[plen-1] == '\r' || new_pass[plen-1] == '\n' || new_pass[plen-1] == ' ')) {
            new_pass[--plen] = '\0';
        }
        if (strlen(new_pass) == 4) {
            room_control_change_password(room, new_pass);
            snprintf(tx_buffer, sizeof(tx_buffer), "PASS CHANGED\r\n");
        } else {
            snprintf(tx_buffer, sizeof(tx_buffer), "INVALID PASSWORD\r\n");
        }
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 1000);
    } else if (strncmp(clean_cmd, "FORCE_FAN:", 10) == 0) {
        // Copia y limpia el argumento
        char arg[4];
        strncpy(arg, clean_cmd + 10, sizeof(arg) - 1);
        arg[sizeof(arg) - 1] = '\0';
        // Elimina espacios y saltos de línea al final
        int alen = strlen(arg);
        while (alen > 0 && (arg[alen-1] == '\r' || arg[alen-1] == '\n' || arg[alen-1] == ' ')) {
            arg[--alen] = '\0';
        }
        int level = atoi(arg);
        if (level >= 0 && level <= 3) {
            room_control_force_fan_level(room, (fan_level_t)level);
            snprintf(tx_buffer, sizeof(tx_buffer), "FAN FORCED TO %d\r\n", level);
        } else {
            snprintf(tx_buffer, sizeof(tx_buffer), "INVALID FAN LEVEL\r\n");
        }
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 1000);
    } else {
        snprintf(tx_buffer, sizeof(tx_buffer), "UNKNOWN COMMAND\r\n");
        HAL_UART_Transmit(&huart3, (uint8_t*)tx_buffer, strlen(tx_buffer), 1000);
    }
}