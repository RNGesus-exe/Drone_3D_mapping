#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <SD_MMC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define MESSAGE_SIZE 64
#define TIMESTAMP_SIZE 20

extern TaskHandle_t server_task;
extern TaskHandle_t camera_task;

extern volatile char shared_data[MESSAGE_SIZE];
extern char date_time[TIMESTAMP_SIZE];
extern portMUX_TYPE data_mutex;
extern SemaphoreHandle_t task_camera_ready;
extern SemaphoreHandle_t task_server_ready;

extern const char *COMMANDS[];
extern const int TOTAL_COMMANDS;

extern const char *ACK_MSG;

enum COMMAND
{
    SCAN,
    ACQUIRE,
    STOP
};

#endif // GLOBALS_H
