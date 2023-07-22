#ifndef CAMERA_H
#define CAMERA_H

#include "globals.h"
#include "sdcard.h"

extern int frame_quality;        // 0-100 [less compressed - more compressed]
extern size_t frames_to_capture; // If you want to capture more frames then use this function

// Camera task function declaration
void cameraTask(void *pvParameters);

bool initCamera();

bool capturePhotoAndSave(const char *mode, int quantity);

#endif // CAMERA_H
