#ifndef SDCARD_H
#define SDCARD_H

#include "globals.h"

extern int64_t curr_ind;
extern int64_t total_size;

bool setupSDCard();

bool readConfig();

bool writeConfig();

#endif // SDCARD_H
