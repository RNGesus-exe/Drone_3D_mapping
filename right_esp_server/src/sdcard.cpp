#include "sdcard.h"

int64_t curr_ind;
int64_t total_size;

bool setupSDCard()
{
    return SD_MMC.begin() ? true : false;
}

bool readConfig()
{
    File file = SD_MMC.open("/Config/config.txt", FILE_READ);
    if (!file)
    {
        return false;
    }
    if (!file.available())
    {
        file.close();
        return false;
    }
    curr_ind = file.parseInt();
    if (!file.available())
    {
        file.close();
        return false;
    }
    total_size = file.parseInt();
    file.close();

    return true;
}

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