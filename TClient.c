#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "TClient.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        perror("usage TClient ip_address port_num\n");
        exit(-1);
    }

    //Open client
    printf("Welcome to Travel Agency!\n");
    printf("=========================\n");

    int client_fd;
    int portNum = atoi(argv[2]);
    int BUFFER_SIZE = 256;

    //buffer =User Input, commands is for tokens
    char buffer[BUFFER_SIZE], **commands = (char**) malloc(sizeof(char) * 3);
    //Make memory space for user command tokens
    for(int i = 0; i < 3; i++)
        commands[i] = (char*) malloc(sizeof(char) * BUFFER_SIZE);

    //client socket address info
    struct sockaddr_in serv_addr;
    //stores host information
    struct hostent *server;
    //LOGON and LOGOFF
    printf("client: LOGON <username> or LOGOFF to close.\n");
    do
    {
        //display
        printf("client: ");
        //zero both buffers
        memset(buffer,0,BUFFER_SIZE);
        for(int i = 0; i < 3; i++)
            memset(commands[i],0,BUFFER_SIZE);
        //get command line
        fgets(buffer, BUFFER_SIZE - 1, stdin);
        //tokenize it
        setCommands(commands, buffer, BUFFER_SIZE);
        //check user logon or off
        if(strcmp(commands[0], "LOGON") == 0)
        {
            if(strlen(commands[1]) > 0)
            {
                printf("client: Logging on to server.\n");
                break;
            }
            printf("client: Missing Username! Try again.\n");
        }
        else if(strcmp(commands[0], "LOGOFF") == 0)
        {
            printf("client: Exiting server.\n");
            for(int i = 0; i < 3; i++)
                free(commands[i]);
            free(commands);
            exit(0);
        }
        //if not found try again
        else
        {
            printf("client: Command not found! Try again.\n");
        }
    }
    while(1);
    //logged on, starting client
    //Open socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    //check if successful
    if (client_fd < 0)
    {
        perror("Can't open socket.");
        exit(-1);
    }
    //connect to server
    server = gethostbyname(argv[1]);
    //check if successful
    if (server == NULL)
    {
        printf("Host not found\n");
        exit(0);
    }
    //initialize struct
    memset((char *) &serv_addr,0, sizeof(serv_addr));
    //set family
    serv_addr.sin_family = AF_INET;
    // copy the network address to sockaddr_in structure
    memmove((char *) &serv_addr.sin_addr.s_addr,(char *)server->h_addr_list[0],  server->h_length);
    //set port
    serv_addr.sin_port = htons(portNum);
    //if can't connect, close
    if (connect(client_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("Can't connect");
        exit(-1);
    }

    int n = write(client_fd, commands[1], strlen(commands[1]));
    if (n < 0)
    {
        perror("Could not write to socket");
        exit(-1);
    }

    printf("client: Logged on to server.\n");
    //Server mode
    serverProcess(client_fd, buffer, BUFFER_SIZE, commands);

    for(int i = 0; i < 3; i++)
        free(commands[i]);
    free(commands);

    close(client_fd);
    return 0;
}

void setCommands(char** commands, char* buffer, int BUFFER_SIZE)
{
    for(int i = 0; i < 3; i++)
        memset(commands[i],0,BUFFER_SIZE);

    sscanf(buffer, "%[^' '] %[^' '] %[^'\n']", commands[0], commands[1], commands[2]);
    for(int i = 0; i < 3; i++)
        if( commands[i][strlen(commands[i]) -1] == '\n')
            commands[i][strlen(commands[i]) -1]= '\0';
}

void serverProcess(int client_fd, char* buffer, int BUFFER_SIZE, char** commands)
{
    do
    {
        printf("client: ");
        memset(buffer,0,BUFFER_SIZE) ;
        for(int i = 0; i < 3; i++)
            memset(commands[i],0,BUFFER_SIZE);

        fgets(buffer, BUFFER_SIZE - 1, stdin);

        setCommands(commands, buffer, BUFFER_SIZE);

        if(strcmp(commands[0], "LOGOFF") == 0)
        {
            printf("client: Goodbye\n");
            int n = write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
            break;
        }
        else if((strcmp(commands[0], "QUERY") == 0) && (commands[1] != NULL) )
        {
            int n = write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
            memset(buffer,0,BUFFER_SIZE);
            n = read(client_fd,buffer, BUFFER_SIZE-1);
            if(n<0)
            {
                perror("Can't read from socket\n");
                exit(-1);
            }
            if(strcmp(buffer, "-1") == 0)
                printf("client: Flight %s is not available.\n", commands[1]);
            else
                printf("client: Flight %s has %s seats available.\n", commands[1], buffer);
        }
        else if(strcmp (commands[0], "RESERVE") == 0)
        {
            if(!(commands[1] && commands[2]))
            {
                printf("Commands missing");
                continue;
            }

            int n = write(client_fd, buffer, BUFFER_SIZE);

            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }

            memset(buffer,0,BUFFER_SIZE);
             n = read(client_fd,buffer, BUFFER_SIZE-1);
            if(n<0)
            {
                perror("Can't read from socket\n");
                exit(-1);
            }
            if(strcmp(buffer, "-1") == 0)
                printf("client: Flight %s is not available.\n", commands[1]);
            else if(strcmp(buffer, "-2") == 0)
                printf("client: Not enough seats available for flight %s.\n", commands[1]);
            else
                printf("client: %s seats bought for flight %s.\n", commands[2],commands[1]);
        }
        else if(strcmp(commands[0], "RETURN") == 0)
        {
            int n = write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
            memset(buffer,0,BUFFER_SIZE);
            n = read(client_fd,buffer, BUFFER_SIZE-1);
            if(n<0)
            {
                perror("Can't read from socket\n");
                exit(-1);
            }
            if(strcmp(buffer, "-1") == 0)
                printf("client: Flight %s is not available.\n", commands[1]);
            else if(strcmp(buffer, "-2") == 0)
                printf("client: Can not refund that many seats for flight %s.\n", commands[1]);
            else
                printf("client: %s seats refund for flight %s.\n", commands[2],commands[1]);
        }
        else if(strcmp(commands[0], "ROUND_TRIP") == 0)
        {
            int n= write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
            memset(buffer,0,BUFFER_SIZE);
            n = read(client_fd,buffer, BUFFER_SIZE-1);
            if(n<0)
            {
                perror("Can't read from socket\n");
                exit(-1);
            }
            if(strcmp(buffer, "-1") == 0)
                printf("client: Round trip %s is not available.\n", commands[1]);
            else if(strcmp(buffer, "-2") == 0)
                printf("client: Not enough seats available for round flight trip: %s.\n", commands[1]);
            else
                printf("client: %s seats bought for round flight trip: %s.\n", commands[2],commands[1]);
        }
        else if(strcmp(commands[0], "RETURN_TRIP") == 0)
        {
            int n= write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
            memset(buffer,0,BUFFER_SIZE);
            n = read(client_fd,buffer, BUFFER_SIZE-1);
            if(n<0)
            {
                perror("Can't read from socket\n");
                exit(-1);
            }
            if(strcmp(buffer, "-1") == 0)
                printf("client: Flight %s is not available.\n", commands[1]);
            else if(strcmp(buffer, "-2") == 0)
                printf("client: Can not refund that many seats for flight %s.\n", commands[1]);
            else
                printf("client: %s seats refund for round flight trip: %s.\n", commands[2],commands[1]);
        }
        else if( (strcmp(commands[0], "LIST") == 0) || (strcmp(commands[0], "LIST_AVAILABLE") == 0) ) //LIST, LIST<>, LIST_AVALIBLE, LIST_AVALIBLE<>.
        {
            int n= write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
            memset(buffer,0,BUFFER_SIZE);
            n = read(client_fd,buffer, BUFFER_SIZE-1);
            if(n<0)
            {
                perror("Can't read from socket\n");
                exit(-1);
            }
            printf("%s", buffer);
        }
        else if( (strcmp(commands[0], "ENTER") == 0 ) && (strcmp(commands[1], "CHAT") == 0 ) && (strlen(commands[2]) != 0) )
        {
            int n= write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
            chatMode(client_fd, buffer, BUFFER_SIZE, commands);
            printf("client:Welcome back to server.\n");
        }
        else
        {
            printf("client: Command not found! Try again.\n");
        }
    }
    while(1);
}

void chatMode(int client_fd, char* buffer, int BUFFER_SIZE, char** commands)
{
    printf("client: Entering Chat Room.\n");
    char name[BUFFER_SIZE];
    strcpy(name,commands[2]);

    pthread_t tid;
    pthread_create(&tid, NULL, readMSG, &client_fd);
    do
    {
        printf("chat %s <<: ", name);
        memset(buffer,0,BUFFER_SIZE);
        for(int i = 0; i < 3; i++)
            memset(commands[i],0,BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);
        setCommands(commands, buffer, BUFFER_SIZE);
        if(strcmp(commands[0], "TEXT") == 0)
        {
            int n= write(client_fd, buffer, BUFFER_SIZE);
            if (n < 0)
            {
                perror("Could not write to socket");
                exit(-1);
            }
        }
        else if(strlen(commands[2]) == 0)
        {
            if( (strcmp (commands[0], "EXIT") == 0) && (strcmp(commands[1], "CHAT") == 0) )
            {
                int n= write(client_fd, buffer, BUFFER_SIZE);
                if (n < 0)
                {
                    perror("Could not write to socket");
                    exit(-1);
                }
                printf("chat: Leaving Chat Room.\n");
                pthread_cancel(tid);
                pthread_join(tid,NULL);
                break;
            }
            else if((strcmp(commands[0], "LIST") == 0) || (strcmp(commands[0], "LIST_ALL") == 0) || (strcmp(commands[0], "LIST_OFFLINE") == 0) )
            {
                int n= write(client_fd, buffer, BUFFER_SIZE);
                if (n < 0)
                {
                    perror("Could not write to socket");
                    exit(-1);
                }
            }
            else
            {
                printf("chat: Command not found! Try again.\n");
            }
        }
        else
        {
            printf("chat: Command not found! Try again.\n");
        }
    }
    while(1);
}

void* readMSG(void* arg)
{
    int client_fd = *((int*) arg);
    int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    do
    {
        memset(buffer,0,BUFFER_SIZE);
        int n = read(client_fd,buffer, BUFFER_SIZE-1);
        if(n<0)
        {
            perror("Can't read from socket\n");
            exit(-1);
        }
        printf("%s", buffer);
    }
    while(1);
    return(NULL);
}
