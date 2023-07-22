#include "globals.h"

TaskHandle_t server_task;
TaskHandle_t camera_task;
volatile char shared_data[MESSAGE_SIZE];
char date_time[TIMESTAMP_SIZE];
portMUX_TYPE data_mutex = portMUX_INITIALIZER_UNLOCKED;
SemaphoreHandle_t task_camera_ready = xSemaphoreCreateBinary();
SemaphoreHandle_t task_server_ready = xSemaphoreCreateBinary();

const char *COMMANDS[] = {"SCAN", "ACQUIRE", "STOP"};
const int TOTAL_COMMANDS = 3;
const char *ACK_MSG = "ACK";