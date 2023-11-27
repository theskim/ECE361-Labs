#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "packet.h"

#define MAX_NAME 20
#define MAX_PASSWORD 30
#define MAX_DATA 100

//hard-coded users:
unsigned char IDs[] = {"Steve","Jill","Grace","Joe"};
unsigned char passwords[] = {"Steve1","Jill2","Grace3","Joe4"};

struct client {
    unsigned char IP[MAX_NAME];
    unsigned int port;
    unsigned int session_ID;
    unsigned char ID[MAX_NAME];
    struct client *next;
    };

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
    };


struct client *head = NULL;

int register_client(unsigned char[] ID, unsigned char[] IP, unsigned int port,unsigned char[] password)
{
    bool flag = false;
    for(int i = 0; i < 4; i++)
    {
        if(!strcmp(IDs[i],ID) && !strcmp(passwods[i], password))
            flag = true;
    }

    if(!flag)
        printf("Invalid credentials");
        return -1
    
    struct client *newNode = malloc(sizeof(struct client));
    newNode->session_ID = -1;
    newNode->ID = ID;
    newNode->IP = IP;
    newNode->port = port;
    newNode->next = NULL;

    struct client *traverser = head;
    while(traverser->next!=NULL && strcmp(traverser->ID,ID))
        traverser = traverser->next;
    
    if(!strcmp(traverser->ID,ID))
    {
        printf("User already registered");
        return -1;
    }
    
    traverser->next = newNode;
    return 1;
}

int remove_client(unsigned char[] IP)
{
    if (head!=NULL)
        if(head->next == NULL && !strcmp(head->IP,IP))
            head = NULL;
            return 1;
        if(head->next == NULL && strcmp(head->IP,IP))
            printf("ID not found");
            return -1;
    else
        printf("List already empty");
        return -1; 
    
    struct client *traverser = head->next;
    struct client *prev_node = head;

    while(traverser->next!=NULL && strcmp(traverser->IP,IP))
        prev_node = prev_node->next;
        traverser = traverser->next;

    if(strcmp(traverser->IP,IP))
        printf("ID not found");
        return -1;

    prev_node->next = traverser->next;
    return 1;
}


int main(int argc, char *argv[]){
    struct sockaddr_in sin;
    char buf[MAX_LINE * 3];
    socklen_t addr_len = sizeof(sin);
    int server_socket;
    char* str_end;

    //use me!
    //https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

    if (argc != 2){
        perror("Should have two arguments");
        exit(1);
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        close(server_socket);
        exit(1);
    }

    bzero(&sin, sizeof(sin)); // fills a buffer with zero bytes
    sin.sin_family = AF_INET; // IPv4
    sin.sin_addr.s_addr = INADDR_ANY; // update address (0.0.0.0)
    long port = strtol(argv[1], &str_end, 10);

    if (errno == ERANGE || str_end == argv[1] || *str_end != '\0'){ // error returned or nothing received
        perror("invalid port");
        close(server_socket);
        exit(1);
    }
    sin.sin_port = htons(port); // Convert values between host and network byte order
    
    if ((bind(server_socket, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("bind");
        close(server_socket);
        exit(1);
    }

    if (recvfrom(server_socket, buf, sizeof(buf), 0, (struct sockaddr*) &sin, &addr_len) < 0){
        perror("recvfrom");
        close(server_socket);
        exit(1);  
    }

    //buf[strlen("ftp")] = '\0'; // safety
    if (!strcmp(buf, "ftp")){
        // reply with yes
        sendto(server_socket, "yes", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin));
    } else {
        // reply with no
        sendto(server_socket, "no", strlen("no"), 0, (struct sockaddr*) &sin, sizeof(sin));
    }
}