#include "socketio.h"

int connectToServer(const char *server_ip, int server_port, const char *signature)
{
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("%s FAILURE: Failed to create socket", signature);
        return -1;
    }

    // Server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &(serverAddr.sin_addr)) <= 0)
    {
        printf("%s FAILURE: Invalid server address", signature);
        close(sockfd);
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("%s FAILURE: Failed to connect to the server", signature);
        close(sockfd);
        return -1;
    }

    // Send Date&Time to server
    char timestamp[DATE_TIME_SIZE] = {};
    getCurrentDateTime(timestamp);
    sendDataToServer(timestamp, DATE_TIME_SIZE, sockfd, signature);
    printf("%s: Date and Time = %s sent to server\n", signature, timestamp);

    return sockfd;
}

void sendDataToServer(const char *buffer, size_t size, const int sockfd, const char *signature)
{
    // Send data to the server
    ssize_t bytesSent = send(sockfd, buffer, size, 0);
    if (bytesSent < 0)
    {
        printf("%s FAILURE: Failed to send data to server", signature);
        close(sockfd);
    }
}

void receiveDataFromServer(char *buffer, size_t size, const int sockfd, const char *signature)
{
    // Receive acknowledgement from the server
    memset(buffer, 0, sizeof(buffer)); // clear the buffer
    ssize_t bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytesRead < 0)
    {
        printf("%s FAILURE: Failed to receive data from server", signature);
        close(sockfd);
    }
}

void uint64ToString(uint64_t value, char *buffer, size_t bufferSize)
{
    snprintf(buffer, bufferSize, "%lu", value);
}

bool receiveImage(const int sockfd, const char *signature)
{
    // Receive the ImagePacket struct
    struct ImageData receivedPacket;

    // Receive the timestamp field
    ssize_t bytesRead = recv(sockfd, receivedPacket.timestamp, sizeof(receivedPacket.timestamp), 0);
    if (bytesRead != sizeof(receivedPacket.timestamp))
    {
        printf("%s FAILURE: Failed to receive timestamp field\n", signature);
        close(sockfd);
        return false;
    }

    // Receive the image_id field
    bytesRead = recv(sockfd, &receivedPacket.image_id, sizeof(receivedPacket.image_id), 0);
    if (bytesRead != sizeof(receivedPacket.image_id))
    {
        printf("%s FAILURE: Failed to receive ticks field\n", signature);
        close(sockfd);
        return false;
    }

    // convert int to string
    char id_string[21]; // Assuming the number can be represented in at most 20 digits
    uint64ToString(receivedPacket.image_id, id_string, sizeof(id_string));

    // Receive the ticks field
    bytesRead = recv(sockfd, &receivedPacket.ticks, sizeof(receivedPacket.ticks), 0);
    if (bytesRead != sizeof(receivedPacket.ticks))
    {
        printf("%s FAILURE: Failed to receive ticks field\n", signature);
        close(sockfd);
        return false;
    }

    // Receive the img_length field
    bytesRead = recv(sockfd, &receivedPacket.img_length, sizeof(receivedPacket.img_length), 0);
    if (bytesRead != sizeof(receivedPacket.img_length))
    {
        printf("%s FAILURE: Failed to receive img_length field\n", signature);
        close(sockfd);
        return false;
    }

    printf("ID = %ld, ticks = %ld, timestamp = %s, length = %ld\n", receivedPacket.image_id, receivedPacket.ticks, receivedPacket.timestamp, receivedPacket.img_length);

    // Allocate memory for the image data
    receivedPacket.data = (uint8_t *)malloc(receivedPacket.img_length * sizeof(uint8_t));
    if (receivedPacket.data == NULL)
    {
        printf("%s FAILURE: Failed to allocate memory for image data\n", signature);
        close(sockfd);
        return false;
    }

    // Receive the image data
    size_t totalBytesReceived = 0;
    bytesRead = 0;

    // Loop until all the image data is received
    while (totalBytesReceived < receivedPacket.img_length)
    {
        // Read a chunk of image data from the client
        bytesRead = recv(sockfd, receivedPacket.data + totalBytesReceived, receivedPacket.img_length - totalBytesReceived, 0);

        if (bytesRead < 0)
        {
            printf("%s FAILURE: Failed to receive image data\n", signature);
            free(receivedPacket.data);
            receivedPacket.data = NULL;
            close(sockfd);
            return false;
        }
        else if (bytesRead == 0)
        {
            // Connection closed by the client
            break;
        }
        else
        {
            // Update the total bytes received
            totalBytesReceived += bytesRead;
        }
    }

    if (totalBytesReceived < receivedPacket.img_length)
    {
        printf("%s FAILURE: Failed to receive complete image data\n", signature);
        free(receivedPacket.data);
        receivedPacket.data = NULL;
        close(sockfd);
        return false;
    }

    // Check if directory exists
    char filename[50];
    if (strcmp(signature, "LEFT CLIENT") == 0)
    {
        strcpy(filename, "LEFT_ESP/ESPImages/");
    }
    else
    {
        strcpy(filename, "RIGHT_ESP/ESPImages/");
    }
    strcat(filename, receivedPacket.timestamp);

    // Create the directory if it does not exist
    mkdir(filename, 0777);

    strcat(filename, "/image");
    strcat(filename, id_string);
    strcat(filename, ".jpeg");

    // Store Jpeg Image permanently
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open file for writing: %s\n", filename);
        free(receivedPacket.data);
        receivedPacket.data = NULL;
        return false;
    }

    size_t bytesWritten = fwrite(receivedPacket.data, sizeof(uint8_t), receivedPacket.img_length, file);
    fclose(file);

    if (bytesWritten != receivedPacket.img_length)
    {
        fprintf(stderr, "Failed to write data to file: %s\n", filename);
        remove(filename); // Delete the partially written file
    }

    // Store Jpeg Data permanently
    if (strcmp(signature, "LEFT CLIENT") == 0)
    {
        strcpy(filename, "LEFT_ESP/ESPImagesData/");
    }
    else
    {
        strcpy(filename, "RIGHT_ESP/ESPImagesData/");
    }
    strcat(filename, receivedPacket.timestamp);

    // Create the directory if it does not exist
    mkdir(filename, 0777);

    strcat(filename, "/image");
    strcat(filename, id_string);
    strcat(filename, ".txt");

    file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open file for writing: %s\n", filename);
        free(receivedPacket.data);
        receivedPacket.data = NULL;
        return false;
    }

    fprintf(file, "%s\n", receivedPacket.timestamp);
    fprintf(file, "%lu\n", receivedPacket.image_id);
    fprintf(file, "%lu\n", receivedPacket.ticks);
    fprintf(file, "%lu\n", receivedPacket.img_length);

    printf("SUCCESS: JPEG data saved successfully\n");
    fclose(file);

    // Deallocate Dynamic array
    free(receivedPacket.data);
    receivedPacket.data = NULL;

    // Send acknowledgement that data has been successfully received
    if (send(sockfd, "ACK_L", strlen("ACK_L"), 0) != strlen("ACK_L"))
    {
        perror("There was an error sending the acknowledgement");
        return false;
    }
    return true;
}

bool receiveImages(const int sockfd, const char *signature)
{
    // Receive the ImagePacket struct
    struct ImageData receivedPacket;

    // Receive the timestamp field
    ssize_t bytesRead = recv(sockfd, receivedPacket.timestamp, sizeof(receivedPacket.timestamp), 0);
    if (bytesRead != sizeof(receivedPacket.timestamp))
    {
        printf("%s FAILURE: Failed to receive timestamp field\n", signature);
        close(sockfd);
        return false;
    }

    // Receive the image_id field
    bytesRead = recv(sockfd, &receivedPacket.image_id, sizeof(receivedPacket.image_id), 0);
    if (bytesRead != sizeof(receivedPacket.image_id))
    {
        printf("%s FAILURE: Failed to receive ticks field\n", signature);
        close(sockfd);
        return false;
    }

    // convert int to string
    char id_string[21]; // Assuming the number can be represented in at most 20 digits
    uint64ToString(receivedPacket.image_id, id_string, sizeof(id_string));

    // Receive the ticks field
    bytesRead = recv(sockfd, &receivedPacket.ticks, sizeof(receivedPacket.ticks), 0);
    if (bytesRead != sizeof(receivedPacket.ticks))
    {
        printf("%s FAILURE: Failed to receive ticks field\n", signature);
        close(sockfd);
        return false;
    }

    // Receive the img_length field
    bytesRead = recv(sockfd, &receivedPacket.img_length, sizeof(receivedPacket.img_length), 0);
    if (bytesRead != sizeof(receivedPacket.img_length))
    {
        printf("%s FAILURE: Failed to receive img_length field\n", signature);
        close(sockfd);
        return false;
    }

    printf("Received id = %ld, ticks = %ld, timestamp = %s, length = %ld\n",
           receivedPacket.image_id, receivedPacket.ticks, receivedPacket.timestamp, receivedPacket.img_length);

    // Allocate memory for the image data
    receivedPacket.data = (uint8_t *)malloc(receivedPacket.img_length * sizeof(uint8_t));
    if (receivedPacket.data == NULL)
    {
        printf("%s FAILURE: Failed to allocate memory for image data\n", signature);
        close(sockfd);
        return false;
    }

    // Receive the image data
    size_t totalBytesReceived = 0;
    bytesRead = 0;

    // Loop until all the image data is received
    while (totalBytesReceived < receivedPacket.img_length)
    {
        // Read a chunk of image data from the client
        bytesRead = recv(sockfd, receivedPacket.data + totalBytesReceived, receivedPacket.img_length - totalBytesReceived, 0);

        if (bytesRead < 0)
        {
            printf("%s FAILURE: Failed to receive image data\n", signature);
            free(receivedPacket.data);
            receivedPacket.data = NULL;
            close(sockfd);
            return false;
        }
        else if (bytesRead == 0)
        {
            // Connection closed by the client
            break;
        }
        else
        {
            // Update the total bytes received
            totalBytesReceived += bytesRead;
        }
    }

    if (totalBytesReceived < receivedPacket.img_length)
    {
        printf("%s FAILURE: Failed to receive complete image data\n", signature);
        free(receivedPacket.data);
        receivedPacket.data = NULL;
        close(sockfd);
        return false;
    }

    printf("Data received successfully\n");

    // Check if directory exists
    char filename[50] = "LeftESP/ESPImages/";
    strcat(filename, receivedPacket.timestamp);

    // Create the directory if it does not exist
    mkdir(filename, 0777);

    strcat(filename, "/image");
    strcat(filename, id_string);
    strcat(filename, ".jpeg");

    // Store Jpeg Image permanently
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open file for writing: %s\n", filename);
        free(receivedPacket.data);
        receivedPacket.data = NULL;
        return false;
    }

    size_t bytesWritten = fwrite(receivedPacket.data, sizeof(uint8_t), receivedPacket.img_length, file);
    fclose(file);

    if (bytesWritten != receivedPacket.img_length)
    {
        fprintf(stderr, "Failed to write data to file: %s\n", filename);
        remove(filename); // Delete the partially written file
    }
    else
    {
        printf("SUCCESS: JPEG file saved successfully: %s\n", filename);
    }

    // Store Jpeg Data permanently
    strcpy(filename, "LeftESP/ESPImagesData/");
    strcat(filename, receivedPacket.timestamp);

    // Create the directory if it does not exist
    mkdir(filename, 0777);

    strcat(filename, "/image");
    strcat(filename, id_string);
    strcat(filename, ".txt");

    file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open file for writing: %s\n", filename);
        free(receivedPacket.data);
        receivedPacket.data = NULL;
        return false;
    }

    fprintf(file, "%s\n", receivedPacket.timestamp);
    fprintf(file, "%lu\n", receivedPacket.image_id);
    fprintf(file, "%lu\n", receivedPacket.ticks);
    fprintf(file, "%lu\n", receivedPacket.img_length);

    printf("SUCCESS: JPEG data saved successfully\n");
    fclose(file);

    // Deallocate Dynamic array
    free(receivedPacket.data);
    receivedPacket.data = NULL;

    // Send acknowledgement that data has been successfully received
    if (send(sockfd, "ACK_L", strlen("ACK_L"), 0) != strlen("ACK_L"))
    {
        perror("There was an error sending the acknowledgement");
        return false;
    }
    printf("Acknowledgement has been sent to the client\n");

    return true;
}