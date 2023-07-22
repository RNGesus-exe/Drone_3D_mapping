#include "server.h"

IPAddress staticIP(192, 168, 220, 71); // Desired static IP address
IPAddress gateway(192, 168, 220, 1);   // Gateway IP address
IPAddress subnet(255, 255, 255, 0);    // Subnet mask

WiFiServer server(PORT);
WiFiClient client;

void serverTask(void *pvParameters)
{
    connectToWiFi();
    startServer();

    for (;;)
    {
        if (!client.connected())
        {
            connectToClient();
        }

        if (client.available())
        {
            char client_msg[MESSAGE_SIZE];
            int bytes_received = client.readBytesUntil('\n', client_msg, MESSAGE_SIZE - 1);
            if (bytes_received > 0)
            {
                client_msg[bytes_received] = '\0';
                Serial.print("SUCCESS: Received command = ");
                Serial.println(client_msg);

                // Tokenize the command
                char *client_tokens = strdup(client_msg); // Create a copy of the string
                char *command = strtok(client_tokens, ",");

                if (strcmp(command, COMMANDS[STOP]) == 0)
                {
                    Serial.println("SUCCESS: Connection Terminated");
                    client.stop();
                    continue;
                }

                // Process Command
                processCommand(command, client_msg);
            }
            else
            {
                Serial.println("FAILURE: Failed to receive command from client");
            }
        }
    }
    Serial.println("NOTICE: Server has stopped...");
}

void connectToClient()
{
    // Wait for a new client to connect
    Serial.print("Waiting for client to connect");
    while (!client.connected())
    {
        delay(1000);
        Serial.print(".");
        client = server.available();
        if (client)
        {
            Serial.println("SUCCESS: client connected");
        }
    }

    // Get Date Time
    bool time_flag = getDateAndTime();
    if (!time_flag)
    {
        Serial.println("Time&Date was not attained");
    }
    Serial.println("SUCCESS: Time&Date acquired");
}

bool getDateAndTime()
{
    int bytes_received = 0;
    int total_bytes_received = 0;
    memset(date_time, '\0', TIMESTAMP_SIZE);

    // Receive the date and time string from the server
    do
    {
        bytes_received = client.readBytes(date_time + total_bytes_received, TIMESTAMP_SIZE - total_bytes_received);
        if (bytes_received < 0)
        {
            Serial.println("FAILURE: Failed to receive date and time string from client");
            return false;
        }
        total_bytes_received += bytes_received;
    } while (bytes_received > 0);

    // Print the date and time string to the serial console
    Serial.print("SUCCESS: Date and Time: ");
    Serial.println(date_time);
    return true;
}

bool sendPhotosToServer(const char *mode, int quantity)
{
    // Check if a directory exists
    if (!SD_MMC.exists("/Images") || !SD_MMC.exists("/ImagesData"))
    {
        Serial.println("Directory does not exist");
        return false;
    }

    // Check if demanded images are within criteria
    if (curr_ind >= total_size)
    {
        Serial.println("curr_ind exceeds the total_size");
        return false;
    }

    if (strcmp(mode, "1") == 0)
    {
        for (int i = 0; i < quantity; i++)
        {
            // Check if image exists
            if (!SD_MMC.exists("/Images/image" + String(curr_ind) + ".jpeg") || !SD_MMC.exists("/ImagesData/image" + String(curr_ind) + ".txt"))
            {
                Serial.println("File does not exist");
                return false;
            }

            // Open directory of image
            File pictureFile = SD_MMC.open("/Images/image" + String(curr_ind) + ".jpeg", FILE_READ);
            File pictureData = SD_MMC.open("/ImagesData/image" + String(curr_ind) + ".txt", FILE_READ);
            if (!pictureFile || !pictureData)
            {
                Serial.println("Error opening file for reading");
                return false;
            }

            // Get the length of the image
            size_t fileLength = pictureFile.size();

            // Create a new buffer to store image data
            uint8_t *fileData = new uint8_t[fileLength];
            if (!fileData)
            {
                Serial.println("Failed to allocate memory");
                pictureFile.close();
                return false;
            }

            // Read the file data into the buffer
            pictureFile.read(fileData, fileLength);
            pictureFile.close();

            // Prepare the struct for ImagePacket
            ImageData imageData;
            imageData.image_id = curr_ind;
            imageData.img_length = fileLength;
            imageData.data = fileData;

            // Read the ImageData file (time and ticks)
            if (pictureData && pictureData.available())
            {
                pictureData.readBytesUntil('\n', imageData.timestamp, TIMESTAMP_SIZE);
                if (pictureData.available())
                {
                    imageData.ticks = pictureData.parseInt();
                }
                else
                {
                    Serial.println("Could not read the timestamp");
                    return false;
                }
            }
            else
            {
                Serial.println("Could not read the ticks");
                return false;
            }

            pictureData.close();
            Serial.printf("time = %s, ticks = %lu\n", date_time, imageData.ticks);
            Serial.println("SUCCESS: Picture and Data loaded");

            // Properly convert struct into string
            size_t messageSize = sizeof(imageData.timestamp) + sizeof(imageData.image_id) + sizeof(imageData.ticks) + sizeof(imageData.img_length) + imageData.img_length;
            uint8_t *messageData = new uint8_t[messageSize];

            // Copy the struct fields to the message buffer
            uint8_t *messagePtr = messageData;
            memcpy(messagePtr, imageData.timestamp, sizeof(imageData.timestamp));
            messagePtr += sizeof(imageData.timestamp);
            memcpy(messagePtr, &imageData.image_id, sizeof(imageData.image_id));
            messagePtr += sizeof(imageData.image_id);
            memcpy(messagePtr, &imageData.ticks, sizeof(imageData.ticks));
            messagePtr += sizeof(imageData.ticks);
            memcpy(messagePtr, &imageData.img_length, sizeof(imageData.img_length));
            messagePtr += sizeof(imageData.img_length);
            memcpy(messagePtr, imageData.data, imageData.img_length);

            // Send the struct over Wi-Fi using WiFiClient
            client.write(messageData, messageSize);
            client.flush();
            Serial.println("SUCCESS: Data sent to server");

            // Clean up and close file pointer
            delete[] fileData;
            delete[] messageData;
            pictureFile.close();

            // Receive Acknowledgement from server then delete the picture from sd card
            char buffer[MESSAGE_SIZE] = {};
            int bytes_received = 0;
            int total_bytes_received = 0;
            int send_result = 0;

            // Receive the acknowledgement from the server
            do
            {
                bytes_received = client.readBytes(buffer + total_bytes_received, MESSAGE_SIZE - total_bytes_received);
                if (bytes_received < 0)
                {
                    Serial.println("Failed to receive message from server");
                    break;
                }
                total_bytes_received += bytes_received;
            } while (bytes_received > 0);

            // Check if the response is valid
            if (strcmp(buffer, HANDSHAKE_RESPONSE) == 0)
            {
                Serial.println("SUCCESS: Data correctly received by server");
            }
            else
            {
                Serial.printf("FAILURE: Invalid response from server: %s\n", buffer);
                return false;
            }

            // Delete picture and data from SD Card
            if (!SD_MMC.remove("/Images/image" + String(curr_ind) + ".jpeg"))
            {
                Serial.println("Error deleting file");
                return false;
            }

            if (!SD_MMC.remove("/ImagesData/image" + String(curr_ind) + ".txt"))
            {
                Serial.println("Error deleting file");
                return false;
            }

            Serial.print("SUCCESS: Deleted old image with path = ");
            Serial.println("Image/image" + String(curr_ind) + ".jpeg");

            // Update the curr_ind
            curr_ind++;
        }
    }
    else if (strcmp(mode, "2") == 0)
    {
        // Dump all pictures
        while (curr_ind < total_size)
        {
            // Notify Client another image is being sent
            client.write("CONT", sizeof("CONT"));

            // Check if image exists
            if (!SD_MMC.exists("/Images/image" + String(curr_ind) + ".jpeg") || !SD_MMC.exists("/ImagesData/image" + String(curr_ind) + ".txt"))
            {
                Serial.println("File does not exist");
                return false;
            }

            // Open directory of image
            File pictureFile = SD_MMC.open("/Images/image" + String(curr_ind) + ".jpeg", FILE_READ);
            File pictureData = SD_MMC.open("/ImagesData/image" + String(curr_ind) + ".txt", FILE_READ);
            if (!pictureFile || !pictureData)
            {
                Serial.println("Error opening file for reading");
                return false;
            }

            // Get the length of the image
            size_t fileLength = pictureFile.size();

            // Create a new buffer to store image data
            uint8_t *fileData = new uint8_t[fileLength];
            if (!fileData)
            {
                Serial.println("Failed to allocate memory");
                pictureFile.close();
                return false;
            }

            // Read the file data into the buffer
            pictureFile.read(fileData, fileLength);
            pictureFile.close();

            // Prepare the struct for ImagePacket
            ImageData imageData;
            imageData.image_id = curr_ind;
            imageData.img_length = fileLength;
            imageData.data = fileData;

            // Read the ImageData file (time and ticks)
            if (pictureData && pictureData.available())
            {
                pictureData.readBytesUntil('\n', imageData.timestamp, TIMESTAMP_SIZE);
                if (pictureData.available())
                {
                    imageData.ticks = pictureData.parseInt();
                }
                else
                {
                    Serial.println("Could not read the timestamp");
                    return false;
                }
            }
            else
            {
                Serial.println("Could not read the ticks");
                return false;
            }

            pictureData.close();
            Serial.printf("time = %s, ticks = %lu\n", date_time, imageData.ticks);
            Serial.println("SUCCESS: Picture and Data loaded");

            // Properly convert struct into string
            size_t messageSize = sizeof(imageData.timestamp) + sizeof(imageData.image_id) + sizeof(imageData.ticks) + sizeof(imageData.img_length) + imageData.img_length;
            uint8_t *messageData = new uint8_t[messageSize];

            // Copy the struct fields to the message buffer
            uint8_t *messagePtr = messageData;
            memcpy(messagePtr, imageData.timestamp, sizeof(imageData.timestamp));
            messagePtr += sizeof(imageData.timestamp);
            memcpy(messagePtr, &imageData.image_id, sizeof(imageData.image_id));
            messagePtr += sizeof(imageData.image_id);
            memcpy(messagePtr, &imageData.ticks, sizeof(imageData.ticks));
            messagePtr += sizeof(imageData.ticks);
            memcpy(messagePtr, &imageData.img_length, sizeof(imageData.img_length));
            messagePtr += sizeof(imageData.img_length);
            memcpy(messagePtr, imageData.data, imageData.img_length);

            // Send the struct over Wi-Fi using WiFiClient
            client.write(messageData, messageSize);
            client.flush();
            Serial.println("SUCCESS: Data sent to server");

            // Clean up and close file pointer
            delete[] fileData;
            delete[] messageData;
            pictureFile.close();

            // Receive Acknowledgement from server then delete the picture from sd card
            char buffer[MESSAGE_SIZE] = {};
            int bytes_received = 0;
            int total_bytes_received = 0;
            int send_result = 0;

            // Receive the acknowledgement from the server
            do
            {
                bytes_received = client.readBytes(buffer + total_bytes_received, MESSAGE_SIZE - total_bytes_received);
                if (bytes_received < 0)
                {
                    Serial.println("Failed to receive message from server");
                    break;
                }
                total_bytes_received += bytes_received;
            } while (bytes_received > 0);

            // Check if the response is valid
            if (strcmp(buffer, HANDSHAKE_RESPONSE) == 0)
            {
                Serial.println("SUCCESS: Data correctly received by server");
            }
            else
            {
                Serial.printf("FAILURE: Invalid response from server: %s\n", buffer);
                return false;
            }

            // Delete picture and data from SD Card
            if (!SD_MMC.remove("/Images/image" + String(curr_ind) + ".jpeg"))
            {
                Serial.println("Error deleting file");
                return false;
            }

            if (!SD_MMC.remove("/ImagesData/image" + String(curr_ind) + ".txt"))
            {
                Serial.println("Error deleting file");
                return false;
            }

            Serial.print("SUCCESS: Deleted old image with path = ");
            Serial.println("Image/image" + String(curr_ind) + ".jpeg");

            // Update the curr_ind
            curr_ind++;
        }

        // NOTIFY client not more picture are coming
        client.write("FIN", sizeof("FIN"));
    }
    else
    {
        Serial.println("FAILURE: Invalid mode specified");
        return false;
    }

    // Update the Queue in SD card of esp32
    if (!writeConfig())
    {
        Serial.println("There was an issue updating the config file");
        return false;
    }

    return true;
}

void processCommand(const char *command, const char *client_msg)
{
    /*
    NOTE: SCAN will be handled on CORE 0

    1) SCAN : Take pictures and save in SDcard
        SCAN'\n'MODE'\n'QUANTITY
        MODE = 1 [Take QUANTITY pictures]
        MODE = 2 [TAKE 'n' pictures in QUANTITY seconds]
    2) ACQUIRE : Load pictures from SDcard and send to client
        ACQUIRE'\n'MODE'\n'QUANTITY
        MODE = 1 [Send QUANTITY pictures]
        MODE = 2 [Send All pictures, QUANTITY is a dummy variable here]
    */

    if (strcmp(command, COMMANDS[SCAN]) == 0)
    {
        // Enter Critical Section
        taskENTER_CRITICAL(&data_mutex);
        strcpy((char *)shared_data, client_msg);
        taskEXIT_CRITICAL(&data_mutex);
        Serial.printf("taskCamera has written sharedData %s\n", client_msg);

        // Notify CORE 1
        xSemaphoreGive(task_camera_ready);

        // wait for CORE 0
        xSemaphoreTake(task_server_ready, portMAX_DELAY);

        // Send acknowledgement to client
        if (!client)
        {
            Serial.println("FAILURE: Acknowledgement could not be sent to client");
        }

        if (client.print(ACK_MSG))
        {
            Serial.println("SUCCESS: Acknowledgment sent to client");
        }
        else
        {
            Serial.println("FAILURE: Failed to send acknowledgment to client");
        }
    }
    else if (strcmp(command, COMMANDS[ACQUIRE]) == 0)
    {
        Serial.println("SUCCESS: Acquire invoked");
        char *client_tokens = strdup(client_msg);
        char *command = strtok(client_tokens, ",");
        char *mode = strtok(NULL, ",");
        int quantity = atoi(strtok(NULL, ","));

        Serial.printf("Command = %s, Mode = %s, Quantity = %d\n", command, mode, quantity);

        if (sendPhotosToServer(mode, quantity))
        {
            Serial.println("SUCCESS: Pictures have been sent to client");
        }
        else
        {
            Serial.println("FAILURE: Pictures could not be sent to client");
        }
    }
    else
    {
        Serial.println("FAILURE: Incorrect command received");
    }
}

void startServer()
{
    server.begin();
    Serial.print("SUCCESS: Server started with IP ");
    Serial.println(WiFi.localIP());
}

void connectToWiFi()
{
    Serial.print("Configuring static IP address");
    WiFi.config(staticIP, gateway, subnet);

    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.print("SUCCESS: WiFi connected. IP address: ");
    Serial.println(WiFi.localIP());
}