
#ifndef MESSAGE_H
#define MESSAGE_H
#include <bits/pthreadtypes.h>

typedef unsigned int Type;
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
typedef struct hostent hostent;
typedef struct stat file_stat;
typedef pthread_t Thread;

// Types
#define INVALID 0
#define LOGIN 1
#define LO_ACK 2
#define LO_NAK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NAK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13

#define MAX_LINE 256
#define MAX_NAME 20
#define MAX_DATA 100
#define MAX_PASSWORD 30
#define SAFE_BACKLOG 128

typedef struct message {
    Type type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
} Message;

typedef struct client {
    unsigned char IP[MAX_DATA];
    unsigned int port; 
    unsigned int session_ID;
    unsigned char ID[MAX_NAME];
    unsigned char password[MAX_PASSWORD];
    struct client *next;
} Client;

typedef struct {
    int socket;
    char* ip;
    int port;
} thread_args;

#endif