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
#include "packet.h"

#define MAX_PENDING 5

int main(int argc, char *argv[]){
    struct sockaddr_in sin;
    char buf[MAX_LINE * 3];
    socklen_t addr_len = sizeof(sin);
    int server_socket;
    char* str_end;
    FILE *fp;

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

    buf[strlen("ftp")] = '\0'; // safety
    if (!strcmp(buf, "ftp")){
        // reply with yes
        sendto(server_socket, "yes", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin));
    } else {
        // reply with no
        sendto(server_socket, "no", strlen("no"), 0, (struct sockaddr*) &sin, sizeof(sin));
    }

    int num_packets_ = 0;
    int packet_num_ = 0;

    do {
        int size = 0;
        char* filename;
        char* payload;
        
        do {
            if (recvfrom(server_socket, buf, sizeof(buf), 0, (struct sockaddr*) &sin, &addr_len) < 0){
                perror("recvfrom");
                close(server_socket);
                exit(1);  
            }
            printf("fah: %s\n", buf);
            char* num_packets = strtok(buf, ":");
            printf("num_packets: %s\n", num_packets);
            num_packets_ = atoi(num_packets);
            
            char* packet_num = strtok(NULL, ":");
            printf("packet_num: %s\n", packet_num);
            packet_num_ = atoi(packet_num);

            char* size_ = strtok(NULL, ":");
            printf("size: %s\n", size_);
            size = atoi(size_);

            filename = strtok(NULL, ":");
            printf("filename: %s\n", filename);

            payload = strtok(NULL, ":");

            if ((int)strlen(payload) != size)
                sendto(server_socket, "no", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin)); 
            else
                sendto(server_socket, "yes", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin));
        } while ((int)strlen(payload) != size);

        if (packet_num_ == 0)
            fp = fopen("test3.jpeg", "w");
        else
            fp = fopen("test3.jpeg", "a");

        if (fp == NULL){
            perror("filename");
            close(server_socket);
            exit(1);
        }
        
        fwrite(payload, sizeof(unsigned char), size, fp);
        // for (int i = 0; i < size && size <= MAX_LINE; ++i)
        //     fprintf(fp, "%c", payload[i]);

        fclose(fp);
    } while (packet_num_ < num_packets_ - 1);

    close(server_socket);
}