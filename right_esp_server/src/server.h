#ifndef SERVER_H
#define SERVER_H

#include "globals.h"
#include "sdcard.h"

#define WIFI_SSID "RNGesus"
#define WIFI_PASSWORD "computer"
#define PORT 8000
#define HANDSHAKE_MESSAGE "HSK_L"
#define HANDSHAKE_RESPONSE "ACK_L"

extern IPAddress staticIP; // Desired static IP address
extern IPAddress gateway;  // Gateway IP address
extern IPAddress subnet;   // Subnet mask

struct ImageData
{
    uint8_t timestamp[TIMESTAMP_SIZE]; // 20 size
    uint64_t image_id;
    uint64_t ticks;
    uint64_t img_length;
    uint8_t *data;
};

extern WiFiServer server;
extern WiFiClient client;

void connectToWiFi();

void startServer();

void processCommand(const char *command, const char *client_msg);

bool capturePhotoAndSave(const String &path);

void connectToClient();

bool getDateAndTime();

void serverTask(void *pvParameters);

bool sendPhotosToServer(const char *mode, int quantity);

#endif // SERVER_H
