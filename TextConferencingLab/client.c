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
#include <stdbool.h>
#include "message.h"
#include "helper.h"

long check_login_args_return_port(char* command, char* ID, char* password, char* port_passed){
    char* str_end;

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
    // if something is missing, then we should exit
    if (scanf("%s %s %s %s %s", command, ID, password, IP, port_passed) != 5){
        perror("should have five arguments");
        exit(1);
    }

    hp = gethostbyname(IP); // get host by name
    bzero(&sin, sizeof(sin)); // fills a buffer with zero bytes
    sin.sin_family = AF_INET; // IPv4

    if (hp) 
        bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length); // copy n bytes from src to dest
    else {
        perror("unknown host");
        exit(1);
    }

    long port = check_login_args_return_port(command, ID, password, port_passed);
    
    sin.sin_port = htons(port); // Convert values between host and network byte order

    if ((deliver_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if (connect(deliver_socket, (sockaddr *)&sin, sizeof(sin)) < 0){
        perror("connect");
        close(deliver_socket);
        exit(1);
    }

    Message* message_sent = NULL;
    Message* message_received = NULL;

    do {
        if (message_sent != NULL)  // free the memory to avoid memory leak
            free(message_sent); 
        if (message_received != NULL)
            free(message_received);
        message_sent = NULL;
        message_received = NULL;

        message_sent = malloc(sizeof(Message));
        message_sent->type = LOGIN;
        
        message_sent->size = strlen(password);
        strcpy((char *)message_sent->source, (char *)ID);
        strcpy((char *)message_sent->data, (char *)password);
        char* message_string = get_string_from_message(*message_sent);

        if (write(deliver_socket, message_string, strlen(message_string)) < 0){
            perror("write");
            close(deliver_socket);
            exit(1);
        }

        if (read(deliver_socket, buf, MAX_LINE) < 0){
            perror("read");
            close(deliver_socket);
            exit(1);
        }

        message_received = malloc(sizeof(Message));
        get_message_from_string(buf, message_received);
        print_message(*message_received); // print message

        if (message_received->type == LO_ACK) // login successful
            printf("Registered Successfully.\n"); 
        else if (message_received->type == LO_NAK) // login not successful
            printf("Login was not successful, try it again.\n");
    } while (message_received->type != LO_ACK); // resend it until we get a proper ACK

    while (true){
        // available commands
        // /logout, /joinsession <session ID>, /leavesession, /createsession <session ID>, /list, /quit
        if (message_sent != NULL)  // free the memory to avoid memory leak
            free(message_sent); 
        if (message_received != NULL)
            free(message_received);
        message_sent = NULL;
        message_received = NULL;

        message_sent = malloc(sizeof(Message));
        message_sent->type = INVALID;

        char new_command[MAX_LINE];
        char optional_second_arg[MAX_LINE];

        if (scanf("%s %s", new_command, optional_second_arg) > 2){
            printf("Should have one or two arguments. Try it again..\n");
            continue;
        }

        if (!strcmp(new_command, "/logout")){       
            message_sent->type = EXIT;
            message_sent->size = strlen("l");
            strcpy((char *)message_sent->source, (char *)ID);
            strcpy((char *)message_sent->data, "l");
        }
        else if (!strcmp(new_command, "/joinsession")){ // join session
            message_sent->type = JOIN;
        }
        else if (!strcmp(new_command, "/leavesession")){ // leave session
            message_sent->type = LEAVE_SESS;
        }
        else if (!strcmp(new_command, "/createsession")){ // create session
            message_sent->type = NEW_SESS;
        }
        else if (!strcmp(new_command, "/list")){ // list session
            message_sent->type = QUERY;
            message_sent->size = strlen("q");
            strcpy((char *)message_sent->source, (char *)ID);
            strcpy((char *)message_sent->data, "q");
        }
        else if (!strcmp(new_command, "/quit")){ // quit session
            message_sent->type = EXIT;
        }
        else {
            printf("invalid command, please try it again, %s, %s\n", new_command, optional_second_arg);
            continue;
        }

        char* message_string = get_string_from_message(*message_sent);
        if (write(deliver_socket, message_string, strlen(message_string)) < 0){
            perror("write");
            close(deliver_socket);
            exit(1);
        }

        if (message_sent->type == EXIT) // only exit if user types /quit
            break;

        if (read(deliver_socket, buf, MAX_LINE) < 0){
            perror("read");
            close(deliver_socket);
            exit(1);
        }

        message_received = malloc(sizeof(Message));
        get_message_from_string(buf, message_received);
    }

    free(message_sent);
    printf("Exiting client %s...\n", ID);
}