#include "camera.h"

int frame_quality = 10;       // 0-100 [less compressed - more compressed]
size_t frames_to_capture = 1; // If you want to capture more frames then use this function

void cameraTask(void *pvParameters)
{
    if (!setupSDCard())
    {
        Serial.println("FAILURE: SD card initialization failed");
    }
    Serial.println("SUCCESS: SD card initialized");

    if (!readConfig())
    {
        Serial.println("FAILURE: SD Card configuration could not be loaded");
    }
    Serial.printf("SUCCESS: curr_ind = %d, total_size = %d\n", curr_ind, total_size);

    if (!initCamera())
    {
        Serial.println("FAILURE: Camera initialization failed");
    }
    Serial.println("SUCCESS: Camera initialized");

    for (;;)
    {
        // Wait for CORE 1 to notify
        xSemaphoreTake(task_camera_ready, portMAX_DELAY);
        Serial.println("NOTICE: taskCamera has started...");

        char client_msg[MESSAGE_SIZE];

        // Enter critical section and read the shared data
        taskENTER_CRITICAL(&data_mutex);
        strcpy(client_msg, (const char *)shared_data);
        taskEXIT_CRITICAL(&data_mutex);
        Serial.printf("NOTICE: taskCamera has read sharedData %s\n", client_msg);

        // Tokenize
        char *command = strtok(client_msg, ",");
        char *mode = strtok(NULL, ",");
        int quantity = atoi(strtok(NULL, ","));

        // Act according to the command
        if (strcmp(command, COMMANDS[SCAN]) == 0)
        {
            Serial.println("U WHAT>");
            if (capturePhotoAndSave(mode, quantity))
            {
                Serial.print("SUCCESS: Pictures taken and saved to SD Card");
            }
            else
            {
                Serial.println("FAILURE: Failed to capture & save picture");
            }
        }
        else
        {
            Serial.println("FAILURE: Invalid command appeared in taskCamera");
        }

        // Notify CORE 0 it can start
        xSemaphoreGive(task_server_ready);

        Serial.println("NOTICE: taskCamera went to idle state...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

bool capturePhotoAndSave(const char *mode, int quantity)
{
    // Check if a directory exists
    if (!SD_MMC.exists("/Images"))
    {
        Serial.println("NOTICE: Directory does not exist");
        if (SD_MMC.mkdir("/Images"))
        {
            Serial.println("NOTICE: Directory created");
        }
        else
        {
            Serial.println("FAILURE: Failed to create directory");
            return false;
        }
    }

    // Handle mode and quantity
    if (strcmp(mode, "1") == 0)
    {
        // Loop for quantity iterations
        for (int i = 0; i < quantity; i++)
        {

            File pictureFile = SD_MMC.open("/Images/image" + String(total_size) + ".jpeg", FILE_WRITE);
            if (!pictureFile)
            {
                Serial.println("FAILURE: Error opening file for writing");
                return false;
            }

            // Take picture
            camera_fb_t *picture = esp_camera_fb_get();
            if (!picture)
            {
                Serial.println("FAILURE: Failed to take picture");
                return false;
            }

            // Save picture to SD card
            if (!pictureFile.write(picture->buf, picture->len))
            {
                Serial.println("FAILURE: Failed to save picture to SD card");
                return false;
            }

            pictureFile.close();
            esp_camera_fb_return(picture);

            // Write contents of image to file
            File pictureData = SD_MMC.open("/ImagesData/image" + String(total_size) + ".txt", FILE_WRITE);
            if (!pictureData)
            {
                Serial.println("FAILURE: Failed to create ImagesData file");
                return false;
            }
            pictureData.println(date_time);
            pictureData.println(total_size);
            int64_t ticks = esp_timer_get_time() / 1000000; // Convert form mSec to Sec
            pictureData.println(ticks);
            pictureData.close();

            Serial.printf("Took picture %d\n", total_size);

            // Update Queue size
            total_size++;
        }
    }
    else if (strcmp(mode, "2") == 0)
    {
        // Loop for quantity seconds
        int64_t endTime = esp_timer_get_time() / 1000000 + quantity;
        int i = 0;
        while (esp_timer_get_time() / 1000000 < endTime)
        {

            File pictureFile = SD_MMC.open("/Images/image" + String(total_size) + ".jpeg", FILE_WRITE);
            if (!pictureFile)
            {
                Serial.println("FAILURE: Error opening file for writing");
                return false;
            }

            // Take picture
            camera_fb_t *picture = esp_camera_fb_get();
            if (!picture)
            {
                Serial.println("FAILURE: Failed to take picture");
                return false;
            }

            // Save picture to SD card
            if (!pictureFile.write(picture->buf, picture->len))
            {
                Serial.println("FAILURE: Failed to save picture to SD card");
                return false;
            }

            pictureFile.close();
            esp_camera_fb_return(picture);

            // Write contents of image to file
            File pictureData = SD_MMC.open("/ImagesData/image" + String(total_size) + ".txt", FILE_WRITE);
            if (!pictureData)
            {
                Serial.println("FAILURE: Failed to create ImagesData file");
                return false;
            }
            pictureData.println(date_time);
            pictureData.println(total_size);
            int64_t ticks = esp_timer_get_time() / 1000000; // Convert form mSec to Sec
            pictureData.println(ticks);
            pictureData.close();

            // Update Queue size
            total_size++;
            i++;
        }
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

    config.frame_size = FRAMESIZE_HD;
    config.jpeg_quality = frame_quality;
    config.fb_count = frames_to_capture;

    // Start camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }
    return true;
}
