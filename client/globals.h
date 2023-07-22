#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/types.h>

#define false 0
#define true 1
#define bool int

#define LEFT_ESP_IP "192.168.239.71"
#define LEFT_ESP_PORT 8000
#define RIGHT_ESP_IP "192.168.239.124"
#define RIGHT_ESP_PORT 8000

#define MESSAGE_SIZE 64
#define COMMAND_SIZE 30
#define MODE_SIZE 20
#define QUANTITY_SIZE 20
#define DATE_TIME_SIZE 20

#define HANDSHAKE_MESSAGE "HSK_"
#define HANDSHAKE_RESPONSE "ACK_"

extern const char *COMMANDS[];
extern const int TOTAL_COMMANDS;

extern pthread_mutex_t data_mutex;
extern pthread_cond_t left_esp_cond;
extern pthread_cond_t right_esp_cond;
extern pthread_cond_t main_process_cond;

extern bool left_esp_finished;
extern bool right_esp_finished;

extern char sharedData[MESSAGE_SIZE];

extern void *getCurrentDateTime(char *);

struct ImageData
{
    uint8_t timestamp[DATE_TIME_SIZE]; // 20 size
    uint64_t image_id;
    uint64_t ticks;
    uint64_t img_length;
    uint8_t *data;
};

enum COMMAND
{
    SCAN,
    ACQUIRE,
    STOP,
    MAP
};

#endif /* GLOBALS_H */
