#ifndef _client_
#define _client_
void setCommands(char** commands, char* buffer, int BUFFER_SIZE);
void serverProcess(int sockfd, char* buffer, int BUFFER_SIZE, char** commands);
void chatMode(int sockfd, char* buffer, int BUFFER_SIZE, char** commands);
void* readMSG(void* arg);
#endif
