#ifndef SOCKETIO_H
#define SOCKETIO_H

#include "globals.h"

int connectToServer(const char *server_ip, int server_port, const char *signature);
void sendDataToServer(const char *buffer, size_t size, const int sockfd, const char *signature);
void receiveDataFromServer(char *buffer, size_t size, const int sockfd, const char *signature);
bool receiveImage(const int sockfd, const char *signature);
bool receiveImages(const int sockfd, const char *signature);
void uint64ToString(uint64_t value, char *buffer, size_t bufferSize);

#endif /* SOCKETIO_H */