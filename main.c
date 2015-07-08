#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include<stdio.h>
#include<signal.h>
#include<string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef bool Activity;
#define active true
#define inactive false
#define DEFAULT_ACTIVITY inactive
#define ACTIVE_STRING "active"
#define INACTIVE_STRING "inactive"
#define SET_ACTIVE_COMMAND_LOWER 'a'
#define SET_ACTIVE_COMMAND_UPPER 'A'
#define SET_INACTIVE_COMMAND_LOWER 'i'
#define SET_INACTIVE_COMMAND_UPPER 'I'
#define QUIT_COMMAND_LOWER 'q'
#define QUIT_COMMAND_UPPER 'Q'
#define TO_ADDRESS "127.0.0.1"
#define SOCK_FLG 0
#define SOCK_PROTO 0
#define PORT 3000
#define MESSAGE_SENT_SIZE 2
#define MESSAGE_SENT_ID_INDEX
#define MESSAGE_SENT_ACTIVITY_INDEX
#define MESSAGE_RECEIVED_SIZE 1
#define CONNECTION_ERROR_MESSAGE 1
#define SOCKET_CREATION_ERROR_MESSAGE 1
typedef int IOresult;
#define READ_FAILED -1
#define CONNECTION_FINISHED 0
#define CONNECTION_FAILED -1
#define SOCKET_CREATION_FAILED -1
typedef unsigned char ID;
typedef int Socket;
//typedef struct sockaddr_in Address;

Activity activity = DEFAULT_ACTIVITY;
ID id = 0;
Socket connection = 0;
struct sockaddr_in serverAdress;


char* activityToString(Activity activity) {
	if(activity == inactive)
		return INACTIVE_STRING;
	return ACTIVE_STRING;
}

void prepare_connection() {
	printf("preparing connection\n");
	// address initialisation
	struct  hostent* serverName = gethostbyname(TO_ADDRESS);
	printf("address initialisation\n");
	bzero(&serverAdress, sizeof(serverAdress));
	serverAdress.sin_family = AF_INET;
	//serverAdress.sin_addr.s_addr = inet_addr(TO_ADDRESS);//htonl(INADDR_ANY);//inet_addr(TO_ADDRESS); /* transformation de l’adresse */
	serverAdress.sin_port = htons(PORT);
	bcopy((char *)serverName->h_addr,
		  (char *)&serverAdress.sin_addr.s_addr,
		  serverName->h_length);

	printf("client creation\n");
	//socket creation
	connection = socket(AF_INET, SOCK_STREAM, SOCK_PROTO);
	printf("start connection\n");
	//check if creation succeded
	if(connection == SOCKET_CREATION_FAILED) {
		perror("erreur lors de la creation de la socket");
		exit(SOCKET_CREATION_ERROR_MESSAGE);
	}

	printf("connecting...\n");
	//connection to server
	if(connect(connection,(struct sockaddr*) &serverAdress, sizeof(serverAdress)) < CONNECTION_FAILED) {
		perror("erreur lors de la conneciton au serveur");
		exit(CONNECTION_ERROR_MESSAGE);
	}
	printf("connection started\n");
}

void sendMessage(unsigned char* message, unsigned int size) {
	printf("sending ");
	unsigned int i;
	for(i = 0; i < size; ++i)
		printf("%u ",message[i]);
	printf("\n");
	if(write(connection, message, size) == READ_FAILED) {
		perror((char*)message);
	}
}

void initializeId() {
	char message[MESSAGE_RECEIVED_SIZE];
	read(connection,message,MESSAGE_RECEIVED_SIZE);
	id = message[0];
	printf("l'id du robot est %d\n",id);
}

void start_connection() {
	prepare_connection();
	initializeId();
}

void disconnect() {
	close(connection);
}

void setActivity(const Activity newValue) {
	printf("activity become%s\n", activityToString(newValue));
	//on envoie la requete au serveur
	unsigned char message[MESSAGE_SENT_SIZE];
	message[0] = id;
	message[1] = newValue;
	printf("id : %u, newValue : %u",id,newValue);
	sendMessage(message,MESSAGE_SENT_SIZE);
	//on modifie la valeure actuelle
	activity = newValue;
}

//return false if command say to stop
bool runCommand(char command) {
	printf("commande : %c\n",command);
	switch(command) {
	case SET_ACTIVE_COMMAND_LOWER:
	case SET_ACTIVE_COMMAND_UPPER:
		setActivity(active);
		break;
	case SET_INACTIVE_COMMAND_LOWER:
	case SET_INACTIVE_COMMAND_UPPER:
		setActivity(inactive);
		break;
	case QUIT_COMMAND_LOWER:
	case QUIT_COMMAND_UPPER:
		return false;
	default:
		printf("commande inconue\n");
		break;
	}
	return true;
}

char getNextCommand() {
	printf("the robot is %s\n",activityToString(activity));
	printf("enter a to activate, i to desactivate, or q to exit\n");
	char toReturn;
	scanf(" %c", &toReturn);
	return toReturn;
}

void runCommands() {
	while(true)
		if(runCommand(getNextCommand()) == false)
			break;
}

void runClient() {
	start_connection();
	runCommands();
	disconnect();
}

int main(void) {
	runClient();
	return 0;
}
