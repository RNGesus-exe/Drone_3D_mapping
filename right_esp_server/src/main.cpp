#include "globals.h"
#include "server.h"
#include "camera.h"

const uint32_t STACK_SIZE = 10000;

void setup()
{
  Serial.begin(115200);

  // Create tasks for server and camera
  /*
    The GPIO + Camera tasks will be handled on core 0
    1) Initialize SD card
    2) Load the configuration of Queue from /Config/config.txt
    3) Initialize Camera
    4) Wait for command from [CORE 1]
    5) After receiving command from [CORE 1], start working on it
    6) After finishing processing, send the results to [CORE 0]
    7) Repeat from [STEP 4]

    The Server and Client communication will be handled on core 1
    1) Connect to WiFi
    2) Start Server
    3) Connect to Client
    4) Receive Date and Time from Client
    5) Wait for command from Client
    6) Receive command from Client
    7) If [SCAN] then notify [CORE 0] to start scanning using [shared variable]
      7.1) Wait for [CORE 0] to finish processing
      7.2) Receive acknowledgement from [CORE 0]
    8) else if [ACQUIRE] start sending the pictures to Client
     8.1) Delete old pictures
    9) Send acknowledgement to Client
    10) Repeat from [STEP 6]
  */
  xTaskCreatePinnedToCore(cameraTask, "Camera Task", STACK_SIZE, NULL, 1, &camera_task, 0);
  xTaskCreatePinnedToCore(serverTask, "Server Task", STACK_SIZE, NULL, 1, &server_task, 1);
}

void loop()
{
}
