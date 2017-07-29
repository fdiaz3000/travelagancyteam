#ifndef DUMMY
#define DUMMY
void socketWrite(int socket_fd, char* buffer);
void socketRead(int socket_fd, char* buffer, int bSize);
int randomPort(char* port);
void* clientType1(void* arg);
void* clientType2(void* arg);
void* clientType3(void* arg);

typedef struct{
	char* host;
	char* port;
}Dummy;
#endif
