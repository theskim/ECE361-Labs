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

#define MAX_LINE 256

long check_login_args_return_port(char* command, char* ID, char* password, hostent* hp, char* port_passed){
    char* str_end;

    if (!hp){
        perror("unknown host");
        exit(1);
    }

    if (strcmp(command, "/login") ||
        !strlen(ID) ||
        !strlen(password) ||
        !strlen(port_passed)){
        perror("first command is not /login");
        exit(1);
    }

    long port = strtol(port_passed, &str_end, 10);
    if (errno == ERANGE || str_end == port_passed || *str_end != '\0'){
        perror("invalid port");
        exit(1);
    }

    return port;
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
    sin.sin_family = AF_INET;
    if (hp)
        bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    
    long port = check_login_args_return_port(command, ID, password, hp, port_passed);
    sin.sin_port = htons(port); // Convert values between host and network byte order

    if ((deliver_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    char register_request[MAX_LINE];
    strcat(register_request,ID);
    strcat(register_request,":");
    strcat(register_request,password);

    if (connect(deliver_socket, (sockaddr*)&sin, sizeof(sin)) < 0){
        perror("connect");
        close(deliver_socket);
        exit(1);
    }

    if (write(deliver_socket, register_request, strlen(register_request)) < 0){
        perror("write");
        close(deliver_socket);
        exit(1);
    }

    buf[strlen("registration_successful")] = '\0'; // safety
    if (!strcmp(buf, "registration_successful")){ // buf == "registration_successful"
        printf("Registered Successfully.\n"); //LO_ACK 
    } else {
        printf("%s\n", buf); // LO_NAK
        close(deliver_socket);
        exit(0);
    }
}