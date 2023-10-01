#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h> 

#define MAX_LINE 256

int main(int argc, char * argv[]){
    if (argc != 3){
        perror("Should have two arguments");
        exit(1);
    }

    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int deliver_socket;
    struct stat buffer;
    socklen_t addr_len = sizeof(sin);

    hp = gethostbyname(argv[1]);
    if (!hp){
        perror("unknown host");
        exit(1);
    }

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(atoi(argv[2]));

    if ((deliver_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    char str1[MAX_LINE];
    char str2[MAX_LINE];
    char filepath[MAX_LINE] = "./";
    if (scanf("%s %s", str1, str2) != 2){
        perror("should have two arguments");
        close(deliver_socket);
        exit(1);
    }

    if (strcmp(str1, "ftp") != 0){
        perror("first arg should be ftp");
        close(deliver_socket);
        exit(1);
    }
    strcat(filepath, str2);

    if (stat(filepath, &buffer) != 0){
        perror("file does not exist");
        close(deliver_socket);
        exit(1);  
    }

    sendto(deliver_socket, "ftp", strlen("ftp"), 0, (struct sockaddr*) &sin, sizeof(sin));
    if (recvfrom(deliver_socket, buf, sizeof(buf), 0, (struct sockaddr*) &sin, &addr_len) < 0){
        perror("recvfrom");
        close(deliver_socket);
        exit(1);  
    }
    
    buf[addr_len] = '\0';
    printf("Server responded: %s\n", buf);

    if (!strcmp(buf, "yes")){ // buf == "yes"
        printf("A file transfer can start.\n");  
    } else {
        close(deliver_socket);
        exit(0);
    }

    close(deliver_socket);
}