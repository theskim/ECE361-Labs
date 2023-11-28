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

long return_port(char* port_passed){
    char* str_end;

    // check if port is valid
    long port = strtol(port_passed, &str_end, 10);
    if (errno == ERANGE || str_end == port_passed || *str_end != '\0'){
        perror("invalid port");
        exit(1);
    }

    return port;
}

void check_login_args(char* command, char* ID, char* password){
    char* str_end;

    // check if user inputted wrong args
    if (strcmp(command, "/login") ||
        !strlen(ID) ||
        !strlen(password)){
        perror("first command is not /login");
        exit(1);
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

    Message* message_sent = NULL;
    Message* message_received = NULL;

    do {
        clear_buffer(buf); // clear buffer
        if (message_sent != NULL)  // free the memory to avoid memory leak
            free(message_sent); 
        if (message_received != NULL)
            free(message_received);
        message_sent = NULL;
        message_received = NULL;

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

        long port = return_port(port_passed);
        sin.sin_port = htons(port); // Convert values between host and network byte order
        check_login_args(command, ID, password);

        if ((deliver_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            exit(1);
        }

        if (connect(deliver_socket, (sockaddr *)&sin, sizeof(sin)) < 0){
            perror("connect");
            close(deliver_socket);
            exit(1);
        }

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
            printf("%s. Login was not successful, try it again.\n", message_received->data);
    } while (message_received->type != LO_ACK); // resend it until we get a proper ACK

    while (true){
        // available commands
        // /logout, /joinsession <session ID>, /leavesession, /createsession <session ID>, /list, /quit
        clear_buffer(buf); // clear buffer
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
        char user_input[MAX_LINE]; // we need fgets because we need one or two arguments not a set number of inputs like scanf
        if (fgets(user_input, sizeof(user_input), stdin) == NULL)
            continue;

        int num_args = sscanf(user_input, "%s %s", new_command, optional_second_arg);
        if (num_args == 0){
            printf("No argument entered.\n");
            continue;
        } else if (num_args > 2){
            printf("Too many arguments.\n");
            continue;
        }

        if (!strcmp(new_command, "/logout")){       
            if (num_args == 2){ // logout has one argument
                printf("Too many arguments.\n");
                continue;
            } 
            message_sent->type = EXIT;
            message_sent->size = strlen("l");
            strcpy((char *)message_sent->source, (char *)ID);
            strcpy((char *)message_sent->data, "l");
        }
        else if (!strcmp(new_command, "/joinsession")){ // join session
            if (num_args == 1){ // joinsession has two arguments
                printf("Not enough arguments.\n");
                continue;
            } 
            message_sent->type = JOIN;
            // TODO
        }
        else if (!strcmp(new_command, "/leavesession")){ // leave session
            if (num_args == 2){ // leavesession has one argument
                printf("Too many arguments.\n");
                continue;
            } 
            message_sent->type = LEAVE_SESS;
            // TODO
        }
        else if (!strcmp(new_command, "/createsession")){ // create session           
            if (num_args == 1){ // createsession has two arguments
                printf("Not enough arguments.\n");
                continue;
            } 
            message_sent->type = NEW_SESS;
            // TODO
        }
        else if (!strcmp(new_command, "/list")){ // list session
            if (num_args == 2){ // list has one argument
                printf("Too many arguments.\n");
                continue;
            } 
            message_sent->type = QUERY;
            message_sent->size = strlen("q");
            strcpy((char *)message_sent->source, (char *)ID);
            strcpy((char *)message_sent->data, "q");
        }
        else if (!strcmp(new_command, "/quit")){ // quit session
            if (num_args == 2){ // quit has one argument
                printf("Too many arguments.\n");
                continue;
            } 
            message_sent->type = EXIT;
        }
        else {
            printf("invalid command, please try it again...\n");
            continue;
        }

        if ((deliver_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            exit(1);
        }

        if (connect(deliver_socket, (sockaddr *)&sin, sizeof(sin)) < 0){
            perror("connect");
            close(deliver_socket);
            exit(1);
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