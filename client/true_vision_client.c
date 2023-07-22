#define _GNU_SOURCE
#include "globals.h"
#include "left_esp_client.h"
#include "right_esp_client.h"

void getUserInput(char *input_buffer);

int main()
{
    pthread_t left_client, right_client;
    cpu_set_t cpuset1, cpuset2;

    // Thread attributes
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Create thread 1 and assign it to core 0
    CPU_ZERO(&cpuset1);
    CPU_SET(0, &cpuset1);
    pthread_create(&left_client, &attr, leftESPClient, NULL);
    pthread_setaffinity_np(left_client, sizeof(cpu_set_t), &cpuset1);

    // Create thread 2 and assign it to core 1
    CPU_ZERO(&cpuset2);
    CPU_SET(1, &cpuset2);
    pthread_create(&right_client, &attr, rightESPClient, NULL);
    pthread_setaffinity_np(right_client, sizeof(cpu_set_t), &cpuset2);

    // Destroy the thread attributes object
    pthread_attr_destroy(&attr);

    // Main process takes input from the user and passes it to the threads
    while (true)
    {
        // Take input from user
        char input[MESSAGE_SIZE];
        getUserInput(input);

        // Check if both threads (esp) have finished their processing
        pthread_mutex_lock(&data_mutex);
        // Write to sharedVariable and notify the threads that new data is available
        strcpy(sharedData, input);
        pthread_mutex_unlock(&data_mutex);

        pthread_cond_signal(&left_esp_cond);
        pthread_cond_signal(&right_esp_cond);
        printf("MAIN PROCESS: Clients have been notified...\n");

        pthread_mutex_lock(&data_mutex);
        while (left_esp_finished == false || right_esp_finished == false)
        {
            pthread_cond_wait(&main_process_cond, &data_mutex);
        }
        printf("MAIN PROCESS: Clients have finished task...\n");
        // Reset the counters
        left_esp_finished = false;
        right_esp_finished = false;
        pthread_mutex_unlock(&data_mutex);
    }
}

void getUserInput(char *input_buffer)
{
    memset(input_buffer, '\0', MESSAGE_SIZE);

    bool correct_command;
    char command[COMMAND_SIZE];
    char mode[MODE_SIZE];
    char quantity[QUANTITY_SIZE];

    do
    {
        correct_command = false;
        do
        {
            printf("\n=========================INPUT====================\n");
            printf("Enter command = ");
            scanf("%s", command);

            // VALIDATE THAT THE COMMAND GIVEN IS CORRECT OR NOT
            for (int i = 0; i < TOTAL_COMMANDS; i++)
            {
                if (strcmp(command, COMMANDS[i]) == 0)
                {
                    correct_command = true;
                    break;
                }
            }

            if (!correct_command)
            {
                printf("FAILURE: The command %s is not valid, please enter again\n", command);
            }
        } while (!correct_command);

        correct_command = false;
        do
        {
            if (strcmp(command, COMMANDS[MAP]) == 0)
            {
                printf("Enter the timestamp(YYYY-MM-DD HH:MM:SS) = ");
            }
            else
            {
                printf("Enter mode = ");
            }
            scanf("%s", mode);

            if (strcmp(mode, "1") == 0 || strcmp(mode, "2") == 0)
            {
                correct_command = true;
                break;
            }

            if (!correct_command)
            {
                printf("FAILURE: The mode %s is not valid, please enter again\n", mode);
            }
        } while (!correct_command);

        if (strcmp(command, COMMANDS[MAP]) == 0)
        {
            // If command is to perform Depth Mapping
            performDepthEstimation(mode);
        }
    } while (strcmp(command, COMMANDS[MAP]) == 0);

    correct_command = false;
    do
    {
        printf("Enter quantity (1 to n) = ");
        scanf("%s", quantity);

        if (atoi(quantity) > 0)
        {
            correct_command = true;
            break;
        }

        if (!correct_command)
        {
            printf("FAILURE: The quantity %s is not valid, please enter again\n", quantity);
        }
    } while (!correct_command);

    // Concatenate all 3 strings
    snprintf(input_buffer, MESSAGE_SIZE, "%s,%s,%s", command, mode, quantity);

    // Append a '\n' to end of string
    // Find the current length of the buffer
    size_t length = strlen(input_buffer);

    // Check if there is enough space to append the newline character
    if (length + 1 < MESSAGE_SIZE)
    {
        // Append the newline character
        input_buffer[length] = '\n';
    }
    else
    {
        printf("FAILURE: Newline appending failed\n");
    }
}
