#ifndef _server_
#define _server_
#include "c_hashmap/hashmap.h"
#include "c_hashmap/hashmap.c"

//args
typedef struct{
    char* ip_address;
    char* start_port;
    int no_ports;
    char* in_filename;
    char* out_filename;
    //int* ports;
} InputArg;

//Flight
typedef struct{
	char* FlightName;
	int AvailableSeats;
	int MaxSeats;
}FlightInfo;

//Client
typedef struct{
	char* name;
	char* handleName;
	int connected;
	int inChat;
	int beenInchat;
	int sockfd;
	pthread_t tid;
}ClientInfo;

pthread_mutex_t mutexFlight;
pthread_mutex_t mutexClient;
pthread_mutex_t mutexExtra;

pthread_cond_t Extra;
int ON;

map_t flight_map;
map_t client_map;

int getPort(char* arg, int displace);

void getCommands(char** cmpBuffer, char* buffer, int BUFFER_SIZE);
void freeCommands(char** cmpBuffer);
void commandsZero(char** cmpBuffer,int BUFFER_SIZE);

void sWrite(int sockfd, char* buffer);
void sRead(int sockfd, char* buffer, int BUFFER_SIZE);

void readInput(char * fileName, map_t flight_map);
void writeOutput(char * fileName, map_t flight_map);

int checkSeats(map_t flight_map, char * flightName);
int reserveSeats(map_t flight_map, char * flightName, int toBuy );
int reserveRound(map_t flight_map, char * flightName, int toBuy );
int refundSeats(map_t flight_map, char * flightName, int toRefound );
int refundRound(map_t flight_map, char * flightName, int toRefound );
void listAll(int clientSockfd,char* buffer, int BUFFER_SIZE);
void listN(int clientSockfd,char* buffer, int BUFFER_SIZE, int n);
void listAvailable(int clientSockfd,char* buffer, int BUFFER_SIZE);
void listAvailableN(int clientSockfd,char* buffer, int BUFFER_SIZE, int n);
void* server(void* arg);
void listConnected();
void listInChat();

void* TsocketManager(void* arg);
void* agent(void* arg);
void* client(void* arg);
void cls();

void inChat(ClientInfo* client, char* buffer, char**cmpBuffer, int BUFFER_SIZE);
void writeToChat(ClientInfo* client, char* buffer, int BUFFER_SIZE);
void listInChatClients(ClientInfo* client, int BUFFER_SIZE);
void listAllUsers(ClientInfo* client, int BUFFER_SIZE);
void listOfflineUsers(ClientInfo* client, int BUFFER_SIZE);

void forEachFree(map_t Map);
#endif
