#include "globals.h"

const char *COMMANDS[] = {"SCAN", "ACQUIRE", "STOP", "MAP"};
const int TOTAL_COMMANDS = 4;

pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t left_esp_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t right_esp_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t main_process_cond = PTHREAD_COND_INITIALIZER;

bool left_esp_finished = false;
bool right_esp_finished = false;

char sharedData[MESSAGE_SIZE];

void *getCurrentDateTime(char *buffer)
{
    time_t raw_time;
    struct tm *time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);

    strftime(buffer, DATE_TIME_SIZE, "%Y-%m-%d %H:%M:%S", time_info);
}