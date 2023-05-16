#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <time.h>

#define PORT 8888
#define COMMAND_SIZE 1024
#define BUFFER_SIZE 1024
#define DATE_TIME_SIZE 20

#define HANDSHAKE_MESSAGE "HSK_L"
#define HANDSHAKE_RESPONSE "ACK_L"
#define HANDSHAKE_COMPLETE "RDY_L"

#define false 0
#define true 1
#define bool int

struct ImageData
{
    uint8_t timestamp[DATE_TIME_SIZE]; // 20 size
    uint64_t image_id;
    uint64_t ticks;
    uint64_t img_length;
    uint8_t *data;
};

//========================= COMMANDS ======================================//

const char *COMMANDS[] = {"SCAN", "ACQUIRE", "STOP"};
const int TOTAL_COMMANDS = 3;

enum commands
{
    SCAN,
    ACQUIRE,
    STOP
};

enum modes
{
    SINGLE, // One picture
    BURST,  // n pictures
    TIME    // m pictures in n amount of time
};

int create_socket();

struct sockaddr_in create_address();

int executeHandshake(int client_socket);

void uint64ToString(uint64_t value, char *buffer, size_t bufferSize);

void bind_socket(int sock, struct sockaddr_in address);

void listen_socket(int sock);

int accept_connection(int sock, struct sockaddr_in *client_address);

bool handle_client(int client_socket);

char *getCurrentDateTime();

bool sendDateAndTime(int client_socket);

bool receiveImage(int client_socket);

int main()
{
    int sock = create_socket();
    struct sockaddr_in address = create_address();
    bind_socket(sock, address);
    listen_socket(sock);

    while (1)
    {
        printf("Waiting for incoming connections...\n");
        struct sockaddr_in client_address;
        int client_sock = accept_connection(sock, &client_address);
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        if (!executeHandshake(client_sock))
        {
            perror("Handshake was not successful, Restarting server");
            continue;
        }

        if (!sendDateAndTime(client_sock))
        {
            perror("Date and Time was not sent, Restarting server");
            continue;
        }

        while (1)
        {
            if (!handle_client(client_sock))
            {
                break;
            }

            if (recv(client_sock, NULL, 0, MSG_PEEK | MSG_DONTWAIT) == 0)
            {
                printf("Connection closed by client. Waiting for new connection...\n");
                close(client_sock);
                break;
            }
        }
    }

    return 0;
}

int create_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

bool sendDateAndTime(int client_socket)
{
    char *date_time = getCurrentDateTime();
    int msg_len = strlen(date_time);
    if (send(client_socket, date_time, msg_len, 0) != msg_len)
    {
        perror("Time&Date not sent correctly");
        return false;
    }
    printf("Time&Date sent successfully to client\n");
    return true;
}

bool receiveImage(int client_socket)
{
    // Receive the ImagePacket struct
    struct ImageData receivedPacket;

    // Receive the timestamp field
    ssize_t bytesRead = recv(client_socket, receivedPacket.timestamp, sizeof(receivedPacket.timestamp), 0);
    if (bytesRead != sizeof(receivedPacket.timestamp))
    {
        perror("Failed to receive timestamp field");
        close(client_socket);
        return false;
    }

    // Receive the image_id field
    bytesRead = recv(client_socket, &receivedPacket.image_id, sizeof(receivedPacket.image_id), 0);
    if (bytesRead != sizeof(receivedPacket.image_id))
    {
        perror("Failed to receive ticks field");
        close(client_socket);
        return false;
    }

    // convert int to string
    char id_string[21]; // Assuming the number can be represented in at most 20 digits
    uint64ToString(receivedPacket.image_id, id_string, sizeof(id_string));

    // Receive the ticks field
    bytesRead = recv(client_socket, &receivedPacket.ticks, sizeof(receivedPacket.ticks), 0);
    if (bytesRead != sizeof(receivedPacket.ticks))
    {
        perror("Failed to receive ticks field");
        close(client_socket);
        return false;
    }

    // Receive the img_length field
    bytesRead = recv(client_socket, &receivedPacket.img_length, sizeof(receivedPacket.img_length), 0);
    if (bytesRead != sizeof(receivedPacket.img_length))
    {
        perror("Failed to receive img_length field");
        close(client_socket);
        return false;
    }

    printf("Received id = %ld, ticks = %ld, timestamp = %s, length = %ld\n",
           receivedPacket.image_id, receivedPacket.ticks, receivedPacket.timestamp, receivedPacket.img_length);

    // Allocate memory for the image data
    receivedPacket.data = (uint8_t *)malloc(receivedPacket.img_length * sizeof(uint8_t));
    if (receivedPacket.data == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for image data\n");
        close(client_socket);
        return false;
    }

    // Receive the image data
    size_t totalBytesReceived = 0;
    bytesRead = 0;

    // Loop until all the image data is received
    while (totalBytesReceived < receivedPacket.img_length)
    {
        // Read a chunk of image data from the client
        bytesRead = recv(client_socket, receivedPacket.data + totalBytesReceived, receivedPacket.img_length - totalBytesReceived, 0);

        if (bytesRead < 0)
        {
            perror("Failed to receive image data");
            free(receivedPacket.data);
            receivedPacket.data = NULL;
            close(client_socket);
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
        fprintf(stderr, "Failed to receive complete image data\n");
        free(receivedPacket.data);
        receivedPacket.data = NULL;
        close(client_socket);
        return false;
    }

    // Store Jpeg Image permanently
    char filename[50] = "ESPImages/image";
    strcat(filename, id_string);
    strcat(filename, ".jpg");

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
    strcpy(filename, "ESPImagesData/image");
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
    if (send(client_socket, HANDSHAKE_RESPONSE, strlen(HANDSHAKE_RESPONSE), 0) != strlen(HANDSHAKE_RESPONSE))
    {
        perror("There was an error sending the acknowledgement");
        return false;
    }
    printf("Acknowledgement has been sent to the client\n");
    return true;
}

void uint64ToString(uint64_t value, char *buffer, size_t bufferSize)
{
    snprintf(buffer, bufferSize, "%lu", value);
}

bool handle_client(int client_sock)
{
    bool correct_command = false;

    char buffer[COMMAND_SIZE];
    do
    {
        memset(buffer, '\0', COMMAND_SIZE);
        printf("Enter command to send to server = ");
        scanf("%s", buffer);

        // VALIDATE THAT THE COMMAND GIVEN IS CORRECT OR NOT
        for (int i = 0; i < COMMAND_SIZE; i++)
        {
            if (strcmp(buffer, COMMANDS[i]) == 0)
            {
                correct_command = true;
                break;
            }
        }

        if (!correct_command)
        {
            printf("The command %s is not valid, please enter again\n", buffer);
        }
    } while (!correct_command);

    int bytes_sent = send(client_sock, buffer, strlen(buffer), 0);
    if (bytes_sent == -1)
    {
        perror("There was an issue sending the command to the client");
        return false;
    }

    printf("SUCCESS: %d bytes were sent successfully\n", bytes_sent);

    // SCAN
    if (strcmp(buffer, COMMANDS[SCAN]) == 0)
    {
        memset(buffer, 0, sizeof(buffer)); // clear the buffer
        if (read(client_sock, buffer, BUFFER_SIZE) == 0)
        {
            perror("Connection Lost\n");
            return false;
        }

        if (strcmp(buffer, HANDSHAKE_RESPONSE) != 0)
        {
            perror("ESP could not save photo successfully...\n");
            return false;
        }
        printf("SUCCESS: ESP has saved photo and info successfully\n");
    }
    else if (strcmp(buffer, COMMANDS[ACQUIRE]) == 0) // ACQUIRE
    {
        return receiveImage(client_sock);
    }
    else if (strcmp(buffer, COMMANDS[STOP]) == 0)
    {
        return false;
    }

    return true;
}

int accept_connection(int sock, struct sockaddr_in *client_address)
{
    int addrlen = sizeof(*client_address);
    int client_sock = accept(sock, (struct sockaddr *)client_address, (socklen_t *)&addrlen);
    if (client_sock < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    return client_sock;
}

void listen_socket(int sock)
{
    int listen_result = listen(sock, 3);
    if (listen_result < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
}

void bind_socket(int sock, struct sockaddr_in address)
{
    int bind_result = bind(sock, (struct sockaddr *)&address, sizeof(address));
    if (bind_result < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

struct sockaddr_in create_address()
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    return address;
}

bool executeHandshake(int client_socket)
{
    // Receive Handshake from client
    char buffer[BUFFER_SIZE];          // Used to store contents from client side
    memset(buffer, 0, sizeof(buffer)); // clear the buffer
    if (read(client_socket, buffer, BUFFER_SIZE) == 0)
    {
        perror("Connection Lost\n");
        return false;
    }

    if (strcmp(buffer, HANDSHAKE_MESSAGE) != 0)
    {
        perror("Handshake initiation client not successful...\n");
        return false;
    }
    printf("Initial handshake check was received from client\n");

    // Send Acknowledgement to the client
    if (send(client_socket, HANDSHAKE_RESPONSE, strlen(HANDSHAKE_RESPONSE), 0) != strlen(HANDSHAKE_RESPONSE))
    {
        perror("There was an error sending the acknowledgement");
        return false;
    }
    printf("Acknowledgement has been sent to the client\n");

    // Receive Handshake completion from client
    memset(buffer, 0, sizeof(buffer)); // clear the buffer
    if (read(client_socket, buffer, BUFFER_SIZE) == 0)
    {
        perror("Connection Lost\n");
        return false;
    }

    if (strcmp(buffer, HANDSHAKE_COMPLETE) != 0)
    {
        perror("Handshake with client not successful...\n");
        return false;
    }
    printf("Handshake completed with client\n");
    return true;
}

char *getCurrentDateTime()
{
    time_t raw_time;
    struct tm *time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);

    static char buffer[DATE_TIME_SIZE];
    strftime(buffer, DATE_TIME_SIZE, "%Y-%m-%d %H:%M:%S", time_info);
    return buffer;
}