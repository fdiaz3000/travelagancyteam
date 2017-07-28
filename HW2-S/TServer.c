#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "TServer.h"


int main(int argc, char *argv[])
{
    printf("Welcome to Travel Agent\n");
    // if not enough arguments
    if (argc < 6)
    {
        perror("usage: TServer ip_address start_port no_ports in_file out_file\n");
    }

    //Get input args
    InputArg *serverArguments = (InputArg*)malloc(sizeof(InputArg));
    serverArguments->ip_address = argv[1];
    serverArguments->start_port = argv[2];
    serverArguments->no_ports = atoi(argv[3]);
    serverArguments->in_filename = argv[4];
    serverArguments->out_filename = argv[5];

    //create stuff
    pthread_mutex_init(&mutexFlight,NULL);
    pthread_mutex_init(&mutexClient,NULL);
    pthread_mutex_init(&mutexViper,NULL);
    pthread_mutex_init(&mutexClientCount,NULL);
    pthread_cond_init(&condClientCount, NULL);
    ON = 1;

    pthread_cond_init(&viper, NULL);

    // initialize flight map
    flight_map = hashmap_new();
    //map for clients
    client_map = hashmap_new();
    //initalize hashmap with data from txt
    readInput(serverArguments->in_filename, flight_map);
    //initialize
    pthread_t tidC;
    pthread_t tidSM;
    pthread_create(&tidC, NULL, server, serverArguments);
    pthread_create(&tidSM, NULL, TsocketManager, serverArguments);

    pthread_join(tidC, NULL);
    pthread_join(tidSM, NULL);
    //Destroy Stuff
    pthread_mutex_destroy(&mutexFlight);
    pthread_mutex_destroy(&mutexClient);
    pthread_mutex_destroy(&mutexViper);
    pthread_cond_destroy(&viper);
    pthread_mutex_destroy(&mutexClientCount);
    pthread_cond_destroy(&condClientCount);

    //free maps
    forEachFree(flight_map);
    forEachFree(client_map);
    //Done
    printf("Closed Server, goodbye.\n");
    pthread_exit(NULL);
}


void forEachFree(map_t Map)
{

    hashmap_map* map = (hashmap_map*) Map;
    int index;
    for (index = 0; index < map->table_size; index++)
    {
        if (map->data[index].in_use != 0)
        {
            free(map->data[index].key);
            free(map->data[index].data);
        }
    }
}

int getPort(char* arg, int i)
{
    return atoi(arg) + i;
}

void inputProc(char** cmpBuffer, char* buffer, int BUFFER_SIZE)
{
    cmpBzero(cmpBuffer, BUFFER_SIZE);
    sscanf(buffer, "%[^' '] %[^' '] %[^'\n']", cmpBuffer[0], cmpBuffer[1], cmpBuffer[2]);
    for(int i = 0; i < 3; i++)
        if( cmpBuffer[i][strlen(cmpBuffer[i]) -1] == '\n')
            cmpBuffer[i][strlen(cmpBuffer[i]) -1]= '\0';
}
void inputProc2(char** cmpBuffer, char* buffer, int BUFFER_SIZE)
{
    cmpBzero(cmpBuffer, BUFFER_SIZE);
    sscanf(buffer, "%[^' '] %[^'\n']", cmpBuffer[0], cmpBuffer[1]);
    for(int i = 0; i < 3; i++)
        if( cmpBuffer[i][strlen(cmpBuffer[i]) -1] == '\n')
            cmpBuffer[i][strlen(cmpBuffer[i]) -1]= '\0';
}

void cmpBfree(char** cmpBuffer)
{
    for(int i = 0; i < 3; i++)
        free(cmpBuffer[i]);
    free(cmpBuffer);
}
void cmpBzero(char** cmpBuffer,int BUFFER_SIZE)
{
    for(int i = 0; i < 3; i++)
        memset(cmpBuffer[i],0,BUFFER_SIZE);
}

void sWrite(int sockfd, char* buffer)
{
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("Can't right to socket");
    }
}
void sRead(int sockfd, char* buffer, int BUFFER_SIZE)
{
    memset(buffer, 0, BUFFER_SIZE);
    int n = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (n < 0)
    {
        perror("ERROR reading from socket");
    }
}

void readInput(char * fileName, map_t flight_map)
{
    FILE* file;
    if(!(file = fopen(fileName,"r+")))
    {
        printf("ERROR: Could not open %s\n", fileName);
        return;
    }

    size_t len = 0;
    size_t bytes_read; //ssize_t originally....
    char *in = NULL; // EDIT: Initialized in to null

    while((bytes_read = getline(&in, &len, file)) != -1)
    {
        FlightInfo * flight = (FlightInfo*) malloc(sizeof(FlightInfo));
        flight->FlightName = (char*) calloc(sizeof(char), 8);//will always be XXX-XXX
        char availabeSeats[4];
        char maxSeats[4];

        sscanf(in, "%[^' '] %[^' '] %[^'\n']", flight->FlightName, availabeSeats, maxSeats);
        flight->AvailableSeats = atoi(availabeSeats);
        flight->MaxSeats = atoi(maxSeats);

        printf("Read: '%s', '%d', '%d'\n", flight->FlightName, flight->AvailableSeats, flight->MaxSeats);

        pthread_mutex_lock(&mutexFlight);
        hashmap_put(flight_map,flight->FlightName, flight); //need to check if it worked, later.
        pthread_mutex_unlock(&mutexFlight);
    }
    fclose(file);
}
void writeOutput(char * fileName, map_t flight_map)
{
    FlightInfo* flight;
    int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    FILE* file;
    if(!(file = fopen(fileName,"w")))
    {
        printf("ERROR: Could not open %s\n", fileName);
        return;
    }
    pthread_mutex_lock(&mutexFlight);
    hashmap_map* map = (hashmap_map*) flight_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            flight = (FlightInfo*) map->data[i].data;
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "%s %d %d\n", flight->FlightName, flight->AvailableSeats, flight->MaxSeats);
            fprintf(file, "%s",buffer);
        }
    pthread_mutex_unlock(&mutexFlight);
    fclose(file);
}

void* server(void* arg)
{
    printf("server: server Has Launch!\n");
    InputArg* serverArguments = (InputArg*) arg;
    size_t bytes_read;
    size_t nbytes = 0;
    char *servInput = NULL;
    do
    {
        printf("server: ");
        bytes_read = getline (&servInput, &nbytes, stdin);
        servInput[bytes_read - 1] = '\0';
        if(strcmp(servInput,"EXIT") == 0)
        {
            pthread_mutex_lock(&mutexViper);
            pthread_cond_signal(&viper);
            ON = 0;
            pthread_mutex_unlock(&mutexViper);
            break;
        }
        else if(strcmp(servInput,"LIST") == 0)
        {
            printf("server: Listing all connected users: \n");
            listConnections();
        }
        else if(strcmp(servInput,"LIST CHAT") == 0)
        {
            printf("server: Listing all connected users in chat: \n");
            listInChat();
        }
        else if(strcmp(servInput,"WRITE") == 0)
        {
            printf("server: Writing Data to '%s'.\n",serverArguments->out_filename);
            writeOutput(serverArguments->out_filename,flight_map);
        }
        else
        {
            printf("server: UNKNOWN COMMAND!\n");
        }
    }
    while(1);
    cls();
    printf("server: server Has Terminated!\n");
    pthread_exit(NULL);
}

void listConnections()
{
    ClientInfo* client;
    pthread_mutex_lock(&mutexClient);
    hashmap_map* map = (hashmap_map*) client_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            client = (ClientInfo*) map->data[i].data;
            if(client->connected == 1)
                printf("%s\n",client->name);
        }
    pthread_mutex_unlock(&mutexClient);
}

void listInChat()
{
    ClientInfo* client;
    pthread_mutex_lock(&mutexClient);
    hashmap_map* map = (hashmap_map*) client_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            client = (ClientInfo*) map->data[i].data;
            if( (client->connected == 1) && (client->inChat == 1) )
                printf("%s\n",client->name);
        }
    pthread_mutex_unlock(&mutexClient);
}

void* TsocketManager(void* arg)
{
    InputArg *serverArguments = (InputArg*) arg;
    //create multiple agents to handle each port
    pthread_t tidAgent[serverArguments->no_ports];
    //int ports[serverArguments->no_ports];
    agent_info* agentInfo[serverArguments->no_ports];
    for(int i = 0; i < serverArguments->no_ports; i++)
    {
        //ports[i] = getPort(serverArguments->start_port, i);
        agentInfo[i]->connection_count = 0;
        agentInfo[i]->port_agent = getPort(serverArguments->start_port, i);
        {
            for(int i = 0; i < serverArguments->no_ports; i++)
                pthread_create(&tidAgent[i], NULL, agent, agentInfo[i]);

            pthread_mutex_lock(&mutexViper);
            pthread_cond_wait(&viper, &mutexViper);
            pthread_mutex_unlock(&mutexViper);
            for(int i = 0; i < serverArguments->no_ports; i++)
                pthread_cancel(tidAgent[i]);
            for(int i = 0; i < serverArguments->no_ports; i++)
                pthread_join(tidAgent[i], NULL);
            pthread_exit(NULL);
        }
    }
}

void* agent(void* agent_info_struct)
{
    agent_info* agentInfo = (agent_info*)agent_info_struct;
    int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    int sockfd,
        portno = agentInfo->port_agent,
        optval = 1;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
    }

    memset((char *) &serv_addr,0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    //Eliminates "Address already in use" error from bind.
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval, sizeof(optval)) == -1)
    {
        perror("Couldn't set the socket options!");
        exit(-1);
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind failed");
        exit(-1);
    }

    listen(sockfd,20);
    while(1)
    {
        if(ON == 0)
        {
            break;
        }
        ClientInfo* Client = (ClientInfo*) malloc(sizeof(ClientInfo));
        char* name = (char*) malloc(sizeof(char) * BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);
        memset(name,0,BUFFER_SIZE);
        Client->sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (Client->sockfd < 0)
        {
            perror("ERROR on accept");
            exit(-1);
        }
        sRead(Client->sockfd, buffer, BUFFER_SIZE);
        memcpy(name,buffer,BUFFER_SIZE);
        Client->name = name;
        Client->connected = 1;
        Client->beenInchat = 0;
        Client->inChat = 0;

        pthread_mutex_lock(&mutexClient);
        pthread_create(&Client->tid, NULL, client, Client);
        hashmap_put(client_map, Client->name, Client);
        pthread_mutex_unlock(&mutexClient);

        //Increment the client count and going to sleep if serve 10 clients, 100 clients or 1000 client if the
        //Agent is odd.
        struct timespec timeToWait;
        struct timeval now;
        pthread_mutex_lock(&mutexClientCount);
        agentInfo->connection_count++;
        pthread_mutex_unlock(&mutexClientCount);
        switch(agentInfo->connection_count)
        {
        case 10:    //when the agent has served 10 client.
            timeToWait.tv_sec = now.tv_sec + 5;
            timeToWait.tv_nsec = (now.tv_usec + 1000UL * 100) * 1000UL;
            pthread_cond_timedwait(&condClientCount,&mutexClientCount, &timeToWait);
            break;

        case 100:   //when the agent has served 100 client.
            timeToWait.tv_sec = now.tv_sec+5;
            timeToWait.tv_nsec = (now.tv_usec+1000UL*250)*1000UL;
            pthread_cond_timedwait(&condClientCount,&mutexClientCount, &timeToWait);
            break;

        case 1000:  //send the agent to close the connection if the agent is odd
            if(agentInfo->port_agent & 1)
            {
                close(sockfd);
                break;
            }
        }
    }
    close(sockfd);
    printf("TsocketManager: IN agent: '%d' Has Terminated!\n", portno);
    pthread_exit(NULL);
}

void cls()
{
    ClientInfo* client;
    pthread_mutex_lock(&mutexClient);
    hashmap_map* map = (hashmap_map*) client_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            client = (ClientInfo*) map->data[i].data;
            close(client->sockfd);
            pthread_cancel(client->tid);
            pthread_join(client->tid, NULL);
        }
    pthread_mutex_unlock(&mutexClient);
}

void* client(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    ClientInfo* client = (ClientInfo*) arg;
    int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE],
         **cmpBuffer = (char**) malloc(sizeof(char) * 3);
    for(int i = 0; i < 3; i++)
        cmpBuffer[i] = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    do
    {
        memset(buffer, 0, BUFFER_SIZE);
        cmpBzero(cmpBuffer, BUFFER_SIZE);
        sRead(client->sockfd, buffer, BUFFER_SIZE);
        inputProc(cmpBuffer, buffer, BUFFER_SIZE);
        if(strcmp(cmpBuffer[0], "LOGOFF") == 0)
        {
            client->connected = 0;
            break;
        }
        else if(strcmp(cmpBuffer[0], "QUERY") == 0)
        {
            memset(buffer, 0, BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "%d", checkSeats(flight_map, cmpBuffer[1]));
            sWrite(client->sockfd, buffer);
        }
        else if(strcmp (cmpBuffer[0], "RESERVE") == 0)
        {
            memset(buffer, 0, BUFFER_SIZE);
            int num = reserveSeats(flight_map, cmpBuffer[1],atoi(cmpBuffer[2]));
            snprintf(buffer, BUFFER_SIZE, "%d", num);
            sWrite(client->sockfd, buffer);
        }
        else if(strcmp (cmpBuffer[0], "ROUND_TRIP") == 0)
        {
            memset(buffer, 0, BUFFER_SIZE);
            int num = reserveRound(flight_map, cmpBuffer[1],atoi(cmpBuffer[2]));
            snprintf(buffer, BUFFER_SIZE, "%d", num);
            sWrite(client->sockfd, buffer);
        }
        else if(strcmp(cmpBuffer[0], "RETURN") == 0)
        {
            memset(buffer, 0, BUFFER_SIZE);
            int num = refundSeats(flight_map, cmpBuffer[1],atoi(cmpBuffer[2]));
            snprintf(buffer, BUFFER_SIZE, "%d", num);
            sWrite(client->sockfd, buffer);
        }
        else if(strcmp(cmpBuffer[0], "RETURN_TRIP") == 0)
        {
            memset(buffer, 0, BUFFER_SIZE);
            int num = refundRound(flight_map, cmpBuffer[1],atoi(cmpBuffer[2]));
            snprintf(buffer, BUFFER_SIZE, "%d", num);
            sWrite(client->sockfd, buffer);
        }
        else if( (strcmp(cmpBuffer[0], "LIST") == 0) && (atoi(cmpBuffer[1]) != 0) )
        {
            memset(buffer, 0, BUFFER_SIZE);
            listN(client->sockfd, buffer, BUFFER_SIZE, atoi(cmpBuffer[1]));
        }
        else if(strcmp(cmpBuffer[0], "LIST") == 0)
        {
            memset(buffer, 0, BUFFER_SIZE);
            listAll(client->sockfd, buffer, BUFFER_SIZE);
        }
        else if( (strcmp(cmpBuffer[0], "LIST_AVAILABLE") == 0) && (atoi(cmpBuffer[1]) != 0) )
        {
            memset(buffer, 0, BUFFER_SIZE);
            listAvailableN(client->sockfd, buffer, BUFFER_SIZE, atoi(cmpBuffer[1]));
        }
        else if(strcmp(cmpBuffer[0], "LIST_AVAILABLE") == 0)
        {
            memset(buffer, 0, BUFFER_SIZE);
            listAvailable(client->sockfd, buffer, BUFFER_SIZE);
        }
        else if( (strcmp(cmpBuffer[0], "ENTER") == 0 ) && (strcmp(cmpBuffer[1], "CHAT") == 0 ) )
        {
            char* nick = (char*) malloc(sizeof(char) * BUFFER_SIZE);
            memcpy(nick,cmpBuffer[2], BUFFER_SIZE);
            client->nickName = nick;
            client->beenInchat = 1;
            client->inChat = 1;
            inChat(client,  buffer, cmpBuffer, BUFFER_SIZE);
            client->inChat = 0;
        }
        else
        {
            sWrite(client->sockfd,"client: UNKNOWN COMMAND! Try again.\n");
        }
    }
    while(1);
    close(client->sockfd);
    //printf("client: client Has Terminated!\n");
    pthread_exit(NULL);
}


//Client map manipulation functions
int checkSeats(map_t flight_map, char * flightName)
{
    int check = 0;
    FlightInfo* flight;
    pthread_mutex_lock(&mutexFlight);
    check = hashmap_get(flight_map, flightName, (void**) &flight);
    pthread_mutex_unlock(&mutexFlight);
    if(check != MAP_OK)
        return -1;//-1: flight not available
    return flight->AvailableSeats;
}

int reserveSeats(map_t flight_map, char * flightName, int seatsRequested )
{
    int check = 0;
    int code = 0;
    FlightInfo* flight;
    pthread_mutex_lock(&mutexFlight);
    check = hashmap_get(flight_map, flightName, (void**) &flight);
    if (check != MAP_OK)
    {
        pthread_mutex_unlock(&mutexFlight);
        return -1;
    }
    if( (flight->AvailableSeats - seatsRequested) < 0 )
        code = -2;
    else
        flight->AvailableSeats -= seatsRequested;
    pthread_mutex_unlock(&mutexFlight);
    return code;
}

int reserveRound(map_t flight_map, char * flightName, int seatsRequested )
{
    int check1 = 0, check2 = 0, code = 0, BUFFER_SIZE = 256;
    FlightInfo* flight1, *flight2;
    char buf[BUFFER_SIZE], *buf1, buf2[BUFFER_SIZE], buf3[BUFFER_SIZE];
    memset(buf,0,BUFFER_SIZE);
    memset(buf2,0,BUFFER_SIZE);
    memset(buf3,0,BUFFER_SIZE);
    memcpy(buf,flightName,BUFFER_SIZE);
    buf1 = strtok(buf,"-\n");
    memcpy(buf2,buf1,BUFFER_SIZE);
    buf1 = strtok(NULL,"-\n");
    memcpy(buf3,buf1,BUFFER_SIZE);
    memset(buf,0,BUFFER_SIZE);
    snprintf(buf,BUFFER_SIZE, "%s-%s", buf3, buf2);
    pthread_mutex_lock(&mutexFlight);
    check1 = hashmap_get(flight_map, flightName, (void**) &flight1);
    check2 = hashmap_get(flight_map, buf, (void**) &flight2);
    if ( (check1 != MAP_OK) || (check2!= MAP_OK) )
    {
        pthread_mutex_unlock(&mutexFlight);
        return -1;
    }
    if( ( (flight1->AvailableSeats - seatsRequested) < 0 ) && ( (flight2->AvailableSeats - seatsRequested) < 0 ) )
        code = -2;
    else
    {
        flight1->AvailableSeats -= seatsRequested;
        flight2->AvailableSeats -= seatsRequested;
    }
    pthread_mutex_unlock(&mutexFlight);
    return code;
}

int refundSeats(map_t flight_map, char * flightName, int toRefound )
{
    int check = 0, code = 0;
    FlightInfo* flight;
    pthread_mutex_lock(&mutexFlight);
    check = hashmap_get(flight_map, flightName, (void**) &flight);
    if (check != MAP_OK)
    {
        pthread_mutex_unlock(&mutexFlight);
        return -1;
    }
    if( (flight->AvailableSeats + toRefound) > flight->MaxSeats )
        code =-2;
    else
        flight->AvailableSeats += toRefound;
    pthread_mutex_unlock(&mutexFlight);
    return code;
}
int refundRound(map_t flight_map, char * flightName, int toRefound )
{
    int check1 = 0, check2 = 0, code = 0, BUFFER_SIZE = 256;
    FlightInfo* flight1, *flight2;
    char buf[BUFFER_SIZE], *buf1, buf2[BUFFER_SIZE], buf3[BUFFER_SIZE];
    memset(buf,0,BUFFER_SIZE);
    memset(buf2,0,BUFFER_SIZE);
    memset(buf3,0,BUFFER_SIZE);
    memcpy(buf,flightName,BUFFER_SIZE);
    buf1 = strtok(buf,"-\n");
    memcpy(buf2,buf1,BUFFER_SIZE);
    buf1 = strtok(NULL,"-\n");
    memcpy(buf3,buf1,BUFFER_SIZE);
    memset(buf,0,BUFFER_SIZE);
    snprintf(buf,BUFFER_SIZE, "%s-%s", buf3, buf2);
    pthread_mutex_lock(&mutexFlight);
    check1 = hashmap_get(flight_map, flightName, (void**) &flight1);
    check2 = hashmap_get(flight_map, buf, (void**) &flight2);
    if ( (check1 != MAP_OK) || (check2!= MAP_OK) )
    {
        pthread_mutex_unlock(&mutexFlight);
        return -1;
    }
    if(  ((flight1->AvailableSeats + toRefound) > flight1->MaxSeats) || ( (flight2->AvailableSeats + toRefound) > flight2->MaxSeats) )
        code = -2;
    else
    {
        flight1->AvailableSeats += toRefound;
        flight2->AvailableSeats += toRefound;
    }
    pthread_mutex_unlock(&mutexFlight);
    return code;
}

void listAll(int clientSockfd,char* buffer, int BUFFER_SIZE)
{
    FlightInfo* flight;

    sWrite(clientSockfd,"client: Listing all flights:\n");
    sWrite(clientSockfd,"\tFlight\t\tAvailable Seats\t\tMax Seats\n");
    pthread_mutex_lock(&mutexFlight);
    hashmap_map* map = (hashmap_map*) flight_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            flight = (FlightInfo*) map->data[i].data;
            snprintf(buffer, BUFFER_SIZE, "\t%s\t\t%d\t\t\t%d\n", flight->FlightName, flight->AvailableSeats, flight->MaxSeats);
            sWrite(clientSockfd, buffer);
            memset(buffer, 0, BUFFER_SIZE);
        }
    pthread_mutex_unlock(&mutexFlight);
}
void listN(int clientSockfd,char* buffer, int BUFFER_SIZE, int n)
{
    FlightInfo* flight;
    snprintf(buffer, BUFFER_SIZE, "client: Listing the first %d flights:\n", n);
    sWrite(clientSockfd,buffer);
    memset(buffer, 0, BUFFER_SIZE);
    sWrite(clientSockfd,"\tFlight\t\tAvailable Seats\t\tMax Seats\n");
    pthread_mutex_lock(&mutexFlight);
    hashmap_map* map = ( hashmap_map*) flight_map;
    for (int i = 0,j = 0; (i < map->table_size) && (j < n); i++)
        if (map->data[i].in_use != 0)
        {
            flight = (FlightInfo*) map->data[i].data;
            snprintf(buffer, BUFFER_SIZE, "\t%s\t\t%d\t\t\t%d\n", flight->FlightName, flight->AvailableSeats, flight->MaxSeats);
            sWrite(clientSockfd, buffer);
            memset(buffer, 0, BUFFER_SIZE);
            j++;
        }
    pthread_mutex_unlock(&mutexFlight);
}
void listAvailable(int clientSockfd,char* buffer, int BUFFER_SIZE)
{
    FlightInfo* flight;
    sWrite(clientSockfd,"client: Listing all available flights:\n");
    sWrite(clientSockfd,"\tFlight\t\tAvailable Seats\t\tMax Seats\n");
    pthread_mutex_lock(&mutexFlight);
    hashmap_map* map = ( hashmap_map*) flight_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            flight = (FlightInfo*) map->data[i].data;
            if(flight->AvailableSeats != 0)
            {
                snprintf(buffer, BUFFER_SIZE, "\t%s\t\t%d\t\t\t%d\n", flight->FlightName, flight->AvailableSeats, flight->MaxSeats);
                sWrite(clientSockfd, buffer);
                memset(buffer, 0, BUFFER_SIZE);
            }
        }
    pthread_mutex_unlock(&mutexFlight);
}
void listAvailableN(int clientSockfd,char* buffer, int BUFFER_SIZE, int n)
{
    FlightInfo* flight;
    snprintf(buffer, BUFFER_SIZE, "client: Listing the first %d available flights:\n", n);
    sWrite(clientSockfd,buffer);
    memset(buffer, 0, BUFFER_SIZE);
    sWrite(clientSockfd,"\tFlight\t\tAvailable Seats\t\tMax Seats\n");
    pthread_mutex_lock(&mutexFlight);
    hashmap_map* map = ( hashmap_map*) flight_map;
    for (int i = 0,j = 0; (i < map->table_size) && (j < n); i++)
        if (map->data[i].in_use != 0)
        {
            flight = (FlightInfo*) map->data[i].data;
            if(flight->AvailableSeats != 0)
            {
                snprintf(buffer, BUFFER_SIZE, "\t%s\t\t%d\t\t\t%d\n", flight->FlightName, flight->AvailableSeats, flight->MaxSeats);
                sWrite(clientSockfd, buffer);
                memset(buffer, 0, BUFFER_SIZE);
                j++;
            }
        }
    pthread_mutex_unlock(&mutexFlight);
}

void inChat(ClientInfo* client, char* buffer, char**cmpBuffer, int BUFFER_SIZE)
{
    do
    {
        memset(buffer, 0, BUFFER_SIZE);
        cmpBzero(cmpBuffer, BUFFER_SIZE);
        sRead(client->sockfd, buffer, BUFFER_SIZE);
        inputProc2(cmpBuffer, buffer, BUFFER_SIZE);

        if(strcmp(cmpBuffer[0], "TEXT") == 0)
        {
            writeToAllInChat(client, cmpBuffer[1], BUFFER_SIZE);
        }
        else if(strlen(cmpBuffer[2]) == 0)
        {
            if( (strcmp (cmpBuffer[0], "EXIT") == 0) && (strcmp(cmpBuffer[1], "CHAT") == 0) )
            {
                break;
            }
            else if(strcmp(cmpBuffer[0], "LIST") == 0)
            {
                listInChatToChat(client, BUFFER_SIZE);
            }
            else if(strcmp(cmpBuffer[0], "LIST_ALL") == 0)
            {
                listALLToChat(client, BUFFER_SIZE);
            }
            else if(strcmp(cmpBuffer[0], "LIST_OFFLINE") == 0)
            {
                listOfflineToChat(client, BUFFER_SIZE);
            }
            else
            {
                sWrite(client->sockfd, "CHAT: UNKNOWN COMMAND! Try again.\n");
            }
        }
        else
        {
            sWrite(client->sockfd, "CHAT: UNKNOWN COMMAND! Try again.\n");
        }
    }
    while (1);
}

void writeToAllInChat(ClientInfo* client, char* buffer, int BUFFER_SIZE)
{
    ClientInfo* others;
    char temp[BUFFER_SIZE];
    memset(temp,0,BUFFER_SIZE);
    snprintf(temp, BUFFER_SIZE, "CHAT >> %s: %s\n", client->nickName, buffer);
    pthread_mutex_lock(&mutexClient);
    hashmap_map* map = ( hashmap_map*) client_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            others = (ClientInfo*) map->data[i].data;
            if( (others->inChat == 1) && (strcmp(others->name, client->name) != 0) )
                sWrite(others->sockfd, temp);
        }
    pthread_mutex_unlock(&mutexClient);
}
void listInChatToChat(ClientInfo* client, int BUFFER_SIZE)
{
    ClientInfo* others;
    int count = 0;
    char temp[BUFFER_SIZE];
    sWrite(client->sockfd, "CHAT: Listing current chat users:\n");
    pthread_mutex_lock(&mutexClient);
    hashmap_map* map = ( hashmap_map*) client_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            others = (ClientInfo*) map->data[i].data;
            if(others->inChat == 1)
            {
                memset(temp,0,BUFFER_SIZE);
                snprintf(temp, BUFFER_SIZE, "\t\t%s \n", others->nickName);
                sWrite(client->sockfd, temp);
                count++;
            }
        }
    pthread_mutex_unlock(&mutexClient);
    memset(temp,0,BUFFER_SIZE);
    snprintf(temp, BUFFER_SIZE, "A total of %d users.\n", count);
    sWrite(client->sockfd, temp);
}
void listALLToChat(ClientInfo* client, int BUFFER_SIZE)
{
    ClientInfo* others;
    int count= 0;
    char temp[BUFFER_SIZE];
    sWrite(client->sockfd, "CHAT: Listing all chat users:\n");
    pthread_mutex_lock(&mutexClient);
    hashmap_map* map = ( hashmap_map*) client_map;
    for (int i = 0; i < map->table_size; i++)
        if (map->data[i].in_use != 0)
        {
            others = (ClientInfo*) map->data[i].data;
            if(others->beenInchat == 1)
            {
                memset(temp,0,BUFFER_SIZE);
                snprintf(temp, BUFFER_SIZE, "\t\t%s \n", others->nickName);
                sWrite(client->sockfd, temp);
                count++;
            }
        }
    pthread_mutex_unlock(&mutexClient);
    memset(temp,0,BUFFER_SIZE);
    snprintf(temp, BUFFER_SIZE, "A total of %d users.\n", count);
    sWrite(client->sockfd, temp);
}
void listOfflineToChat(ClientInfo* client, int BUFFER_SIZE)
{
    ClientInfo* others;
    int count = 0;
    char temp[BUFFER_SIZE];

    sWrite(client->sockfd, "CHAT: Listing offline chat users:\n");
    pthread_mutex_lock(&mutexClient);
    hashmap_map* map = (hashmap_map*) client_map;
    for (int i = 0; i < map->table_size; i++)
    {
        if (map->data[i].in_use != 0)
        {
            others = (ClientInfo*) map->data[i].data;
            if( (others->beenInchat == 1) && (others->inChat == 0) )
            {
                memset(temp,0,BUFFER_SIZE);
                snprintf(temp, BUFFER_SIZE, "\t\t%s \n", others->nickName);
                sWrite(client->sockfd, temp);
                count++;
            }
        }
    }
    pthread_mutex_unlock(&mutexClient);
    memset(temp,0,BUFFER_SIZE);
    snprintf(temp, BUFFER_SIZE, "A total of %d users.\n", count);
    sWrite(client->sockfd, temp);
}
