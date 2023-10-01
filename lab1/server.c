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

#define MAX_PENDING 5
#define MAX_LINE 256

int main(int argc, char *argv[]){
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    socklen_t addr_len = sizeof(sin);
    int server_socket;
    char* str_end;

    if (argc != 2){
        perror("Should have two arguments");
        exit(1);
    }

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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

    printf("Bind to Address: %s:%d\n", inet_ntoa(sin.sin_addr), (int)port);

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

    buf[addr_len] = '\0'; // safety
    if (!strcmp(buf, "ftp")){
        // reply with yes
        sendto(server_socket, "yes", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin));
    } else {
        // reply with no
        sendto(server_socket, "no", strlen("no"), 0, (struct sockaddr*) &sin, sizeof(sin));
    }

    close(server_socket);
}