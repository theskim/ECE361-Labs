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

#define MAX_PENDING 5
#define MAX_LINE 256

int main(int argc, char *argv[]){
    if (argc != 2){
        perror("Should have two arguments");
        exit(1);
    }

    struct sockaddr_in sin;
    char buf[MAX_LINE];
    socklen_t addr_len = sizeof(sin);
    int server_socket;

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        close(server_socket);
        exit(1);
    }

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(atoi(argv[1]));
    printf("Bind to Address: %s:%d\n", inet_ntoa(sin.sin_addr), atoi(argv[1]));

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
    printf("Client reponded: %s\n", buf);

    if (!strcmp(buf, "ftp")){
        // reply with yes
        sendto(server_socket, "yes", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin));
    } else {
        // reply with no
        sendto(server_socket, "no", strlen("no"), 0, (struct sockaddr*) &sin, sizeof(sin));
    }

    close(server_socket);
}