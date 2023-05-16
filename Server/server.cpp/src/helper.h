#include <WiFi.h>
#include <WiFiClient.h>
#include <esp32-hal-log.h>
#include <driver/gpio.h>
#include <esp_camera.h>
#include <SD_MMC.h>

#define WIFI_SSID "RNGesus"
#define WIFI_PASSWORD "computer"
#define SERVER_ADDRESS "192.168.113.70"
#define SERVER_PORT 8888

#define SD_CS_PIN 5 // Used with SD Card

#define HANDSHAKE_MESSAGE "HSK_L"
#define HANDSHAKE_RESPONSE "ACK_L"
#define HANDSHAKE_COMPLETE "RDY_L"
#define BUFFER_SIZE 1024
#define TIMESTAMP_SIZE 20

#define COMMAND_SIZE 1024

#define LOOP_DELAY_MS 3000      // delay between loops in milliseconds
#define RECONNECT_DELAY_MS 5000 // delay before attempting to reconnect in milliseconds

//===================== COMMANDS ===========================//
#define SCAN "SCAN"
#define RECEIVE "FRAME"

enum modes
{
    SINGLE,
    BURST,
    TIME
};

struct ImageData
{
    uint8_t timestamp[TIMESTAMP_SIZE]; // 20 size
    uint64_t image_id;
    uint64_t ticks;
    uint64_t img_length;
    uint8_t *data;
};

String server_buf; // Whatever we receive from server, we store here
int server_sz = 0; // We store the length of server message here
String client_buf; // Whatever we send to server from client, we store here
int client_sz = 0; // We store the length of client message here

char date_time[TIMESTAMP_SIZE] = {0}; // The server will send the date and time only once and we will store in this variable

WiFiClient client; // The object which handles the WiFi

int curr_ind = 0;
int total_size = 0;

bool writeConfig()
{
    File file = SD_MMC.open("/Config/config.txt", FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to create config.txt");
        return false;
    }
    file.println(curr_ind);
    file.println(total_size);
    file.close();

    Serial.println("SUCCESS: Updated config.txt");
    return true;
}

bool readConfig()
{
    File file = SD_MMC.open("/Config/config.txt", FILE_READ);
    if (!file)
    {
        Serial.println("Could not read the configuration");
        return false;
    }
    if (!file.available())
    {
        Serial.println("Nothing to read in configuration");
        return false;
    }
    curr_ind = file.parseInt();
    if (!file.available())
    {
        Serial.println("Nothing to read in configuration");
        return false;
    }
    total_size = file.parseInt();
    file.close();

    Serial.printf("SUCCESS: curr_ind = %d, total_size = %d\n", curr_ind, total_size);
    return true;
}

void setupSDCard()
{
    // pinMode(13, INPUT_PULLUP);
    if (!SD_MMC.begin())
    {
        Serial.println("Error initializing SD card");
    }
    Serial.println("SD card initialized");
}

// Send a message to the server and wait for a response
bool handshake()
{
    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = 0;
    int total_bytes_received = 0;
    int send_result = 0;

    // Send the handshake message to the server
    send_result = client.write(HANDSHAKE_MESSAGE, strlen(HANDSHAKE_MESSAGE));
    if (send_result < 0)
    {
        Serial.println("Handshake message failed to send");
        return false;
    }
    Serial.println("Handshake sent to server");

    // Receive the acknowledgement from the server
    do
    {
        bytes_received = client.readBytes(buffer + total_bytes_received, BUFFER_SIZE - total_bytes_received);
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
        Serial.println("Acknowledgement received from server");
    }
    else
    {
        Serial.printf("Invalid response from server: %s\n", buffer);
        return false;
    }

    // Send Ready flag to server
    send_result = client.write(HANDSHAKE_COMPLETE, strlen(HANDSHAKE_COMPLETE));
    if (send_result < 0)
    {
        Serial.println("Handshake complete message failed to send");
        return false;
    }
    Serial.println("Handshake successful with server");

    return true;
}

bool getDateAndTime()
{
    int bytes_received = 0;
    int total_bytes_received = 0;

    // Receive the date and time string from the server
    do
    {
        bytes_received = client.readBytes(date_time + total_bytes_received, TIMESTAMP_SIZE - total_bytes_received);
        if (bytes_received < 0)
        {
            Serial.println("Failed to receive date and time string from server");
            return false;
        }
        total_bytes_received += bytes_received;
    } while (bytes_received > 0);

    // Print the date and time string to the serial console
    Serial.print("Date and Time: ");
    Serial.println(date_time);
    return true;
}

void connectToWiFi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
}
// Connect to Server
void connectToServer()
{
    while (!client.connect(SERVER_ADDRESS, SERVER_PORT))
    {
        Serial.println("Waiting for server connection...");
        delay(1000); // Wait for 1 second before trying again
    }
    // Handshake Protocol
    bool hand_flag = handshake();

    if (!hand_flag)
    {
        Serial.println("Handshake was not successful");
    }

    // Synchronize time
    bool time_flag = getDateAndTime();

    if (!time_flag)
    {
        Serial.println("Time&Date was not attained");
    }

    Serial.println("All checks passed....");
}

bool initCamera()
{
    // Initialize camera
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 90;
    config.fb_count = 2;

    // Start camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }
    return true;
}

bool capturePhotoAndSave(String path)
{
    // Check if a directory exists
    if (!SD_MMC.exists("/Images"))
    {
        Serial.println("Directory does not exist");
        if (SD_MMC.mkdir("/Images"))
        {
            Serial.println("Directory created");
        }
        else
        {
            Serial.println("Failed to create directory");
        }
    }

    // Check if image already exists
    if (SD_MMC.exists("/Images/" + path + ".jpeg"))
    {
        Serial.println("File already exists, deleting...");
        if (!SD_MMC.remove("/Images/" + path + ".jpeg"))
        {
            Serial.println("Error deleting file");
            return false;
        }
    }

    File pictureFile = SD_MMC.open("/Images/" + path + ".jpeg", FILE_WRITE);
    if (!pictureFile)
    {
        Serial.println("Error opening file for writing");
        return false;
    }

    // Take picture
    camera_fb_t *picture = esp_camera_fb_get();
    if (!picture)
    {
        Serial.println("Failed to take picture");
        return false;
    }

    // Save picture to SD card
    if (!pictureFile.write(picture->buf, picture->len))
    {
        Serial.println("Failed to save picture to SD card");
        return false;
    }

    pictureFile.close();
    esp_camera_fb_return(picture);

    // Write contents of image to file
    File pictureData = SD_MMC.open("/ImagesData/" + path + ".txt", FILE_WRITE);
    if (!pictureData)
    {
        Serial.println("Failed to create ImagesData file");
        return false;
    }
    pictureData.println(date_time);
    int64_t ticks = esp_timer_get_time() / 1000000; // Convert form mSec to Sec
    pictureData.println(ticks);
    pictureData.close();

    // Send Acknowledgement to server
    int send_result = client.write(HANDSHAKE_RESPONSE, strlen(HANDSHAKE_RESPONSE));
    if (send_result < 0)
    {
        Serial.println("Save Acknowledgement message failed to send");
        return false;
    }

    // Update the Queue in SD card of esp32
    total_size += 1;
    if (!writeConfig())
    {
        Serial.println("There was an issue updating the config file");
        return false;
    }

    Serial.print("SUCCESS: Picture taken and saved to SD card with id ");
    Serial.println(total_size);

    return true;
}

bool sendPhotosToServer(String path)
{
    // Check if a directory exists
    if (!SD_MMC.exists("/Images") || !SD_MMC.exists("/ImagesData"))
    {
        Serial.println("Directory does not exist");
        return false;
    }

    // Check if image exists
    if (!SD_MMC.exists("/Images/" + path + ".jpeg") || !SD_MMC.exists("/ImagesData/" + path + ".txt"))
    {
        Serial.println("File does not exist");
        return false;
    }

    File pictureFile = SD_MMC.open("/Images/" + path + ".jpeg", FILE_READ);
    File pictureData = SD_MMC.open("/ImagesData/" + path + ".txt", FILE_READ);
    if (!pictureFile || !pictureData)
    {
        Serial.println("Error opening file for reading");
        return false;
    }

    Serial.println("SUCCESS: SD CARD check passed...");

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
    imageData.image_id = curr_ind + 1;
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
    char buffer[BUFFER_SIZE] = {0};
    int bytes_received = 0;
    int total_bytes_received = 0;
    int send_result = 0;

    // Receive the acknowledgement from the server
    do
    {
        bytes_received = client.readBytes(buffer + total_bytes_received, BUFFER_SIZE - total_bytes_received);
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
        Serial.println("Data correctly received by server");
    }
    else
    {
        Serial.printf("Invalid response from server: %s\n", buffer);
        return false;
    }

    // Delete picture and data from SD Card
    if (!SD_MMC.remove("/Images/" + path + ".jpeg"))
    {
        Serial.println("Error deleting file");
        return false;
    }

    if (!SD_MMC.remove("/ImagesData/" + path + ".txt"))
    {
        Serial.println("Error deleting file");
        return false;
    }

    Serial.print("SUCCESS: Deleted old image with path = ");
    Serial.println(path);

    // Update the Queue
    curr_ind++;
    return writeConfig();
}

void cancelConnection()
{
    if (client.connected())
    {
        client.stop();
        Serial.println("Connection with server canceled");
    }
    else
    {
        Serial.println("No active connection to cancel");
    }
}

bool processCommand(const String &cmd)
{
    if (cmd == "SCAN")
    {
        // Take photo and save to SD_CARD, we can also pass the path as an argument
        return capturePhotoAndSave("image" + String(total_size + 1));
    }
    else if (cmd == "ACQUIRE")
    {
        // Send the photos from esp32 ai thinker to server,by dequeuing
        return sendPhotosToServer("image" + String(curr_ind + 1));
    }
    else if (cmd == "STOP")
    {
        cancelConnection();
    }
    Serial.println("The command passed was not correct");
    return false;
}