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
    int buf_len;
    socklen_t addr_len;
    int s;

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        close(s);
        exit(1);
    }

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(atoi(argv[1]));
    printf("Bind to Address: %s:%d\n", inet_ntoa(sin.sin_addr), atoi(argv[1]));

    if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("bind");
        close(s);
        exit(1);
    }

    if ((buf_len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&sin, &addr_len)) <= 0){
        perror("recvfrom");
        close(s);
        exit(1);  
    }

    printf("Server reponded: %s\n", buf);
    if (!strcmp(buf, "ftp")){
        // reply with yes
        sendto(s, "yes", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin));
    } else {
        // reply with no
        sendto(s, "no", strlen("no"), 0, (struct sockaddr*) &sin, sizeof(sin));
    }

    close(s);
}