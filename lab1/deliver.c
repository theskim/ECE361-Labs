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
    char *host = argv[1];
    char buf[MAX_LINE];
    int buf_len;
    int s;
    int len;
    int addr_len;

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr_list[0], (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(atoi(argv[2]));

    /* active open */
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    char str1[MAX_LINE];
    char str2[MAX_LINE];
    char filepath[MAX_LINE] = "./";
    if (scanf("%s %s", str1, str2) != 2){
        perror("should have two arguments");
        exit(1);
    }

    if (strcmp(str1, "ftp") != 0){
        perror("first arg should be ftp");
        exit(1);
    }

    struct stat buffer;
    strcat(filepath, str2);

    if (stat(filepath, &buffer) != 0){
        perror("file does not exist");
        exit(1);  
    }
    
    sendto(s, "ftp", sizeof("ftp"), 0, (struct sockaddr*) &sin, sizeof(sin));

    while (buf_len = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*) &sin, &addr_len)){
        if (!strcmp(buf, "yes")){ // buf == "yes"
            printf("A file transfer can start.");  
        } else {
            close(s);
            exit(0);
        }
    }
}