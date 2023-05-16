#include "helper.h"

void setup()
{
    Serial.begin(115200);
    setupSDCard();
    initCamera();
    connectToWiFi();
    connectToServer();  
    readConfig();
}

void loop()
{
    // Check if the client is still connected
    if (!client.connected())
    {
        Serial.println("Lost connection to server");
        client.stop();
        delay(RECONNECT_DELAY_MS);
        connectToServer();
    }

    // Check for incoming messages from the server
    if (client.available())
    {
        char command[COMMAND_SIZE] = {0};
        int bytes_received = client.readBytesUntil('\0', command, COMMAND_SIZE - 1);
        if (bytes_received > 0)
        {
            command[bytes_received] = '\0';
            processCommand(command);
        }
    }

    // Add a delay to prevent excessive loop iterations
    delay(LOOP_DELAY_MS);
}