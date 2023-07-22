#include "right_esp_client.h"

void *rightESPClient(void *arg)
{
    int sockfd = connectToServer(RIGHT_ESP_IP, RIGHT_ESP_PORT, "RIGHT CLIENT");
    if (sockfd == -1)
    {
        perror("RIGHT CLIENT FAILURE: Connection Failed");
    }
    printf("RIGHT CLIENT: Client Connected to server %s\n", RIGHT_ESP_IP);

    // === MAIN CODE ===
    while (true)
    {
        char buffer[MESSAGE_SIZE] = {};

        // Check if main process has finished writing to sharedVariable
        pthread_mutex_lock(&data_mutex);
        pthread_cond_wait(&right_esp_cond, &data_mutex);
        // Read the shared variable
        strcpy(buffer, sharedData);
        pthread_mutex_unlock(&data_mutex);

        // Check which command main has sent
        char *client_tokens = strdup(buffer);
        char *command = strtok(client_tokens, ",");
        char *mode = strtok(NULL, ",");
        int quantity = atoi(strtok(NULL, ","));

        // Send client_message to the server
        sendDataToServer(buffer, strlen(buffer), sockfd, "RIGHT CLIENT");

        // If ACQUIRE command
        if (strcmp(command, COMMANDS[ACQUIRE]) == 0)
        {
            if (strcmp(mode, "1") == 0)
            {
                printf("RIGHT CLIENT: Receiving %d pictures from server\n", quantity);
                for (int i = 0; i < quantity; i++)
                {
                    receiveImage(sockfd, "RIGHT CLIENT");
                }
            }
            else if (strcmp(mode, "2") == 0)
            {
                printf("RIGHT CLIENT: Receiving all pictures from server\n");
                do
                {
                    receiveDataFromServer(buffer, sizeof(buffer), sockfd, "RIGHT CLIENT");

                    if (strcmp(buffer, "CONT") == 0)
                    {
                        receiveImages(sockfd, "RIGHT CLIENT");
                    }
                    else if (strcmp(buffer, "FIN") == 1)
                    {
                        break;
                    }
                } while (strcmp(buffer, "CONT") == 0);
                printf("Finished receiving all pictures\n");
            }
        }

        right_esp_finished = true;
        while (left_esp_finished == false)
        {
            // Wait for left esp client to finish
        }
        pthread_cond_signal(&main_process_cond);
    }

    // Close the socket
    close(sockfd);

    return NULL;
}