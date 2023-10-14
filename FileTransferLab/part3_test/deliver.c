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
#include <time.h>
#include <math.h>
#include "packet.h"

char *my_itoa(int num, char *str){
    if (str == NULL)
        return NULL;

    sprintf(str, "%d", num);
    return str;
}

int main(int argc, char * argv[]){
    struct hostent *hp;
    struct sockaddr_in sin;
    char buf[MAX_LINE]; // buffer to store result from recvfrom
    int deliver_socket;
    struct stat statbuf; // used to store information about a file or directory
    socklen_t addr_len = sizeof(sin); // ensure that it matches the size of the sin variable
    char* str_end;
    FILE *fp; //pointer to file

    Packet **packets; // array of struct packet
    packets = calloc(100,sizeof(Packet*)); //alocate 100 pointers to packet structs
    
    char payload[3 * MAX_LINE];
    char tmp[MAX_LINE];
    
    int filesize;
    int num_packets;
    int num_bytes;

    if (argc != 3){
        perror("Should have two arguments");
        exit(1);
    }

    hp = gethostbyname(argv[1]);
    if (!hp){
        perror("unknown host");
        exit(1);
    }

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    
    long port = strtol(argv[2], &str_end, 10);
    if (errno == ERANGE || str_end == argv[2] || *str_end != '\0'){
        perror("invalid port");
        exit(1);
    }

    sin.sin_port = htons(port); // Convert values between host and network byte order

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

    if (stat(filepath, &statbuf) != 0){
        perror("file does not exist");
        close(deliver_socket);
        exit(1);  
    }

    // Opening file in reading mode
    fp = fopen(filepath, "rb"); // rb = read binary
 
    if (NULL == fp) {
        printf("file can't be opened \n");
    }

    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    rewind(fp);

    printf("File successfully opened. File size: %d\n", filesize);

    sendto(deliver_socket, "ftp", strlen("ftp"), 0, (struct sockaddr*) &sin, sizeof(sin));
    if (recvfrom(deliver_socket, buf, sizeof(buf), 0, (struct sockaddr*) &sin, &addr_len) < 0){
        perror("recvfrom");
        close(deliver_socket);
        exit(1);  
    }

    buf[strlen("yes")] = '\0'; // safety
    if (!strcmp(buf, "yes")){ // buf == "yes"
        printf("A file transfer can start.\n");  
    } else {
        close(deliver_socket);
        exit(0);
    }

    num_packets = ceil((float) filesize / 1000.0);

    for (int j = 0; j < num_packets; j++){
        if (filesize > MAX_LINE)
            num_bytes = MAX_LINE;        
        else
            num_bytes = filesize;
        
        packets[j] = calloc(1, sizeof(Packet));
        packets[j]->size = num_bytes;
        packets[j]->filename = filepath + 2;
        packets[j]->frag_no = j;
        packets[j]->total_frag = num_packets;
        fread(packets[j]->filedata, 1, num_bytes, fp); // read num_bytes elements of size 1 byte into packet

        my_itoa(num_packets,payload);
        strcat(payload, ":");
        my_itoa(packets[j]->frag_no, tmp);
        strcat(payload, tmp);
        strcat(payload, ":");
        my_itoa(packets[j]->size, tmp);
        strcat(payload, tmp);
        strcat(payload, ":");
        strcat(payload, filepath + 2);
        strcat(payload, ":");

        printf("%s\n", packets[j]->filedata);
        memcpy(payload + strlen(payload), packets[j]->filedata, sizeof(packets[j]->filedata));

        do {
            printf("Sending packet %d (payload sized %d)\n", j, num_bytes);
            sendto(deliver_socket, payload, sizeof(payload), 0, (struct sockaddr*) &sin, sizeof(sin));

            buf[addr_len] = '\0'; // safety

            if (recvfrom(deliver_socket, buf, sizeof(buf), 0, (struct sockaddr*) &sin, &addr_len) < 0){
                perror("recvfrom");
                close(deliver_socket);
                exit(1);  
            }
        } while (strcmp(buf, "yes")); // buf != "yes"

        printf("Packet %d sent\n", j);

        filesize -= num_bytes;
    }

    for (int j = 0; j < num_packets; j++)
        free(packets[j]);
    free(packets);
    
    close(deliver_socket);
}
