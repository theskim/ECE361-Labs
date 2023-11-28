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
#include <pthread.h>
#include "message.h"

// hard-coded users:
unsigned char IDs[][MAX_NAME] = {"Steve", "Jill", "Grace", "Joe"};
unsigned char passwords[][MAX_PASSWORD] = {"Steve1", "Jill2", "Grace3", "Joe4"};

Client *head = NULL;

int register_client(unsigned char ID[], unsigned char IP[], unsigned int port, unsigned char password[]){
    int flag = 0;

    for (int i = 0; i < 4; i++)
        if (!strcmp((char *)IDs[i], (char *)ID) && !strcmp((char *)passwords[i], (char *)password))
            flag = 1;

    if (!flag){
        printf("Invalid credentials");
        return -1;
    }
    
    Client *newNode = malloc(sizeof(Client));
    newNode->session_ID = -1;
    strcpy((char *)newNode->ID, (char *)ID);
    strcpy((char *)newNode->IP, (char *)IP);
    newNode->port = port;
    newNode->next = NULL;

    Client *traverser = head;
    while (traverser->next!=NULL && strcmp((char *)traverser->ID,(char *)ID))
        traverser = traverser->next;
    
    if (!strcmp((char *)traverser->ID, (char *)ID)){
        printf("User already registered");
        return -1;
    }
    
    traverser->next = newNode;
    return 1;
}

int remove_client(unsigned char IP[])
{
    if (head != NULL){
        if (head->next == NULL && !strcmp((char *)head->IP, (char *)IP)){
            head = NULL;
            return 1;
        }
        if (head->next == NULL && strcmp((char *)head->IP, (char *)IP)){
            printf("ID not found");
            return -1;
        }
    } else {
        printf("List already empty");
        return -1; 
    }
    
    Client *traverser = head->next;
    Client *prev_node = head;

    while(traverser->next != NULL && strcmp((char *)traverser->IP, (char *)IP)){
        prev_node = prev_node->next;
        traverser = traverser->next;
    }

    if (strcmp((char *)traverser->IP,(char *)IP)){
        printf("ID not found");
        return -1;
    }

    prev_node->next = traverser->next;
    return 1;
}

void* client_receiver(void* socket_desc){
    int client_socket = *(int*)socket_desc;
    char buf[MAX_DATA];
    int read_size;

    while ((read_size = read(client_socket, buf, MAX_DATA)) > 0){
        buf[read_size] = '\0';
        printf("%s\n", buf);
    }

    return 0;
}

int main(int argc, char *argv[]){
    sockaddr_in sin;
    socklen_t addr_len = sizeof(sin);
    int server_socket;
    char* str_end;
    char buf[MAX_DATA];

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
    
    if ((bind(server_socket, (sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("bind");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, SAFE_BACKLOG) < 0){
        perror("listen");
        close(server_socket);
        exit(1);  
    }

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int client_socket;
    int* new_socket;
    while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len))) {
        printf("Connection accepted\n");

        pthread_t new_thread;
        new_socket = malloc(1);
        *new_socket = client_socket;

        if (pthread_create(&new_thread, NULL, client_receiver, (void*)new_socket) < 0) {
            perror("pthread_create");
            exit(1);
        }

        printf("Assigned Thread\n");
        pthread_join(new_thread, NULL);
    }

    return 0;
}