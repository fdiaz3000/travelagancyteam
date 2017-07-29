#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <netdb.h>
#include "TDummy.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("usage ./%s localhost Data.txt\n", argv[0]);
        exit(0);
    }

    Dummy arg;
    arg.host = argv[1];
    arg.port = argv[2];

    pthread_t tid[10];
    for(int i=0; i<3; i++)
        pthread_create(&tid[i],NULL,clientType1,&arg);
    for(int i=3; i<7; i++)
        pthread_create(&tid[1],NULL,clientType2,&arg);
    for(int i=7; i<10; i++)
        pthread_create(&tid[i],NULL,clientType3,&arg);

    for (int i = 0; i < 10; ++i)
        pthread_join(tid[i],NULL);

    pthread_exit(NULL);
}

void* clientType1(void* arg)
{
    Dummy* serverArguments = (Dummy*)arg;
    int socket_fd,
        portNum = randomPort(serverArguments->port),
        BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Could not open socket.");
        exit(-1);
    }
    server = gethostbyname(serverArguments->host);
    if (server == NULL)
    {
        printf("Host not found\n");
        exit(0);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove( (char *) &serv_addr.sin_addr.s_addr,(char *)server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(portNum);
    if (connect(socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("Could not connect");
        exit(-1);
    }


    socketWrite(socket_fd, "LOGON clientType1");
    sleep(3);
    socketWrite(socket_fd, "LIST");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("%s", buffer);
    sleep(3);
    socketWrite(socket_fd, "LIST 3");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("%s", buffer);
    sleep(3);
    socketWrite(socket_fd, "LIST_AVAILABLE");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("%s", buffer);
    sleep(3);
    socketWrite(socket_fd, "LIST_AVAILABLE 4");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("%s", buffer);
    sleep(3);
    socketWrite(socket_fd, "QUERY MAD-CHI");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: Flight MAD-CHI has %s seats available.\n", buffer);
    socketWrite(socket_fd, "QUERY CHI-MAD");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: Flight CHI-MAD has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "RESERVE_R CHI-MAD 5");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: 5 seats bought for round flight trip: CHI-MAD.\n");
    sleep(3);
    socketWrite(socket_fd, "QUERY MAD-CHI");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: Flight MAD-CHI has %s seats available.\n", buffer);
    socketWrite(socket_fd, "QUERY CHI-MAD");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: Flight CHI-MAD has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "RETURN_R CHI-MAD 2");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: 5 seats bought for round flight trip: CHI-MAD.\n");
    sleep(3);
    socketWrite(socket_fd, "QUERY MAD-CHI");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: Flight MAD-CHI has %s seats available.\n", buffer);
    socketWrite(socket_fd, "QUERY CHI-MAD");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client1: Flight CHI-MAD has %s seats available.\n", buffer);
    sleep(5);
    socketWrite(socket_fd, "ENTER CHAT ClientType1");
    sleep(5);
    socketWrite(socket_fd, "LIST");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("chat1: %s", buffer);
    sleep(5);
    socketWrite(socket_fd, "LIST_ALL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("chat1: %s", buffer);
    sleep(5);
    socketWrite(socket_fd, "LIST_OFFLINE");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("chat1: %s", buffer);
    sleep(5);
    socketWrite(socket_fd,"TEXT HELLO FROM ClientType 1");
    sleep(10);
    socketWrite(socket_fd, "EXIT CHAT");
    sleep(5);
    socketWrite(socket_fd, "LOGOFF");

    close(socket_fd);
    pthread_exit(NULL);
}


void* clientType2(void* arg)
{
    Dummy* serverArguments = (Dummy*)arg;
    int socket_fd,
        portNum = randomPort(serverArguments->port),
        BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Could not open socket");
        exit(-1);
    }
    server = gethostbyname(serverArguments->host);
    if (server == NULL)
    {
        printf("Host not found");
        exit(0);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove( (char *) &serv_addr.sin_addr.s_addr,(char *)server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(portNum);
    if (connect(socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("Could not connect");
        exit(-1);
    }


    socketWrite(socket_fd, "LOGON clientType2");
    sleep(3);
    socketWrite(socket_fd, "QUERY MIA-ORL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: Flight MIA-ORL has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "QUERY ORL-MIA");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: Flight ORL-MIA has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "RESERVE MIA-ORL 50");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: 7 seats bought for flight MIA-ORL.\n");
    sleep(3);
    socketWrite(socket_fd, "QUERY MIA-ORL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: Flight MIA-ORL has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "QUERY ORL-MIA");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: Flight ORL-MIA has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "RETURN MIA-ORL 50");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: 2 seats bought for flight MIA-ORL.\n");
    sleep(3);
    socketWrite(socket_fd, "QUERY MIA-ORL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: Flight MIA-ORL has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "QUERY ORL-MIA");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: Flight ORL-MIA has %s seats available.\n", buffer);
    sleep(5);
    socketWrite(socket_fd, "ENTER CHAT ClientType2");
    sleep(5);
    socketWrite(socket_fd, "LIST");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: %s", buffer);
    sleep(5);
    socketWrite(socket_fd, "TEXT HELLO FROM ClientType2");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: %s", buffer);
    sleep(5);
    socketWrite(socket_fd, "LIST_ALL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: %s", buffer);
    sleep(5);
    socketWrite(socket_fd, "LIST_OFFLINE");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client2: %s", buffer);
    sleep(5);
    socketWrite(socket_fd, "EXIT CHAT");
    sleep(5);
    socketWrite(socket_fd, "LOGOFF");

    close(socket_fd);
    pthread_exit(NULL);
}

void* clientType3(void* arg)
{
    Dummy* serverArguments = (Dummy*)arg;
    int socket_fd,
        portNum = randomPort(serverArguments->port),
        BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Could not open socket.");
        exit(-1);
    }
    server = gethostbyname(serverArguments->host);
    if (server == NULL)
    {
        printf("Host not found\n");
        exit(0);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove( (char *) &serv_addr.sin_addr.s_addr,(char *)server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(portNum);
    if (connect(socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("Could not connect");
        exit(-1);
    }

    socketWrite(socket_fd, "LOGON clientType3");
    sleep(3);
    socketWrite(socket_fd, "QUERY MIA-ORL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: Flight MIA-ORL has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "QUERY ORL-MIA");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: Flight ORL-MIA has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "RESERVE ORL-MIA 5");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: 5 seats bought for flight ORL-MIA.\n");
    sleep(3);
    socketWrite(socket_fd, "QUERY MIA-ORL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: Flight MIA-ORL has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "QUERY ORL-MIA");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: Flight ORL-MIA has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "RETURN ORL-MIA 3");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: 2 seats bought for flight ORL-MIA.\n");
    sleep(3);
    socketWrite(socket_fd, "QUERY MIA-ORL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: Flight MIA-ORL has %s seats available.\n", buffer);
    sleep(3);
    socketWrite(socket_fd, "QUERY ORL-MIA");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("client3: Flight ORL-MIA has %s seats available.\n", buffer);
    sleep(5);
    socketWrite(socket_fd, "ENTER CHAT ClientType3");
    sleep(5);
    socketWrite(socket_fd, "LIST");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("chat: %s", buffer);
    sleep(5);
    socketWrite(socket_fd, "LIST_ALL");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("chat: %s", buffer);
    sleep(7);
    socketWrite(socket_fd, "LIST_OFFLINE");
    socketRead(socket_fd, buffer, BUFFER_SIZE);
    printf("chat: %s", buffer);
    sleep(10);
    socketWrite(socket_fd, "EXIT CHAT");
    sleep(6);
    socketWrite(socket_fd, "LOGOFF");

    close(socket_fd);
    pthread_exit(NULL);
}

int randomPort(char* port)
{
    time_t t;
    srand( (unsigned) time(&t) );
    return (atoi(port)) + (rand() % 8);;
}

void socketWrite(int socket_fd, char* buffer)
{
    int n = write(socket_fd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("Could not write to socket");
        exit(-1);
    }
}

void socketRead(int socket_fd, char* buffer, int BUFFER_SIZE)
{
    memset(buffer, 0, BUFFER_SIZE);
    int n = read(socket_fd, buffer, BUFFER_SIZE);
    if (n < 0)
    {
        perror("Could not read from socket");
        exit(-1);
    }
}
