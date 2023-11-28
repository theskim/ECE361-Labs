#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <errno.h>
#include "message.h"

long check_login_args_return_port(char* command, char* ID, char* password, hostent* hp, char* port_passed){
    char* str_end;

    if (!hp){ // checking if host is known
        perror("unknown host");
        exit(1);
    }

    // check if user inputted wrong args
    if (strcmp(command, "/login") ||
        !strlen(ID) ||
        !strlen(password) ||
        !strlen(port_passed)){
        perror("first command is not /login");
        exit(1);
    }

    // check if port is an integer
    long port = strtol(port_passed, &str_end, 10);
    if (errno == ERANGE || str_end == port_passed || *str_end != '\0'){
        perror("invalid port");
        exit(1);
    }

    return port;
}

char* get_string_from_message(Message message){
    // Format: type:size:source:data 
    char* message_string = malloc(MAX_LINE);
    strcpy(message_string, "");
    sprintf(message_string + strlen(message_string), "%d", (int)message.type);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%d", message.size);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%s", message.source);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%s", message.data);
    
    return message_string;
}

void get_message_from_string(char* string_received, Message* message){
    char* token = strtok(string_received, ":");

    // Format: type:size:source:data -> seperate it by : and store it in message
    for (int i = 0; i <= 3; ++i){
        if (token == NULL)
            break;
        if (i == 0)
            message->type = (Type)atoi(token);
        else if (i == 1)
            message->size = atoi(token);
        else if (i == 2)
            strcpy((char *)message->source, token);
        else if (i == 3)
            strcpy((char *)message->data, token);
        token = strtok(NULL, ":");
    }
}

int main(int argc, char * argv[]){
    // start by connecting/registering. user needs to pass host IP, host port. 
    // Then if successful, register by passing password and id
    hostent *hp;
    sockaddr_in sin;
    char buf[MAX_LINE]; // buffer to store result from recvfrom
    int deliver_socket;
    file_stat statbuf; // used to store information about a file or directory
    socklen_t addr_len = sizeof(sin); // ensure that it matches the size of the sin variable

    char command[MAX_LINE];
    char ID[MAX_LINE];
    char password[MAX_LINE];
    char IP[MAX_LINE];
    char port_passed[MAX_LINE];

    // /login <client ID> <password> <server-IP> <server-port>
    if (scanf("%s %s %s %s %s", command, ID, password, IP, port_passed) != 5){
        perror("should have five arguments");
        //close(deliver_socket);
        exit(1);
    }

    hp = gethostbyname(IP);

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET; // IPv4

    if (hp) bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    long port = check_login_args_return_port(command, ID, password, hp, port_passed);
    
    sin.sin_port = htons(port); // Convert values between host and network byte order

    if ((deliver_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if (connect(deliver_socket, (sockaddr*)&sin, sizeof(sin)) < 0){
        perror("connect");
        close(deliver_socket);
        exit(1);
    }

    Message message;
    message.type = LOGIN;
    message.size = strlen(password);
    strcpy((char *)message.source, (char *)ID);
    strcpy((char *)message.data, (char *)password);
    char* login_packet = get_string_from_message(message);
    printf("%s\n", login_packet);

    if (write(deliver_socket, login_packet, strlen(login_packet)) < 0){
        perror("write");
        close(deliver_socket);
        exit(1);
    }

    if (read(deliver_socket, buf, MAX_LINE) < 0){
        perror("read");
        close(deliver_socket);
        exit(1);
    }

    Message* message_received = malloc(sizeof(Message));
    get_message_from_string(buf, message_received);

    if (message_received->type == LO_ACK){ // login successful
        printf("Registered Successfully.\n"); 
    } else if (message_received->type == LO_NAK){ // login not successful
        printf("Login was not Successful");
        close(deliver_socket);
        exit(0);
    }
}