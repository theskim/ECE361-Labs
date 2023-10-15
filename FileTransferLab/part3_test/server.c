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
#include <assert.h>
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

    int num_packets = 0;
    int packet_num = 0;
    int size = 0;
    int curr_index = 0;
    char filename[MAX_LINE]; 
    char payload[MAX_LINE];   

    do {      
        do {
            if (recvfrom(server_socket, buf, sizeof(buf), 0, (struct sockaddr*) &sin, &addr_len) < 0){
                perror("recvfrom");
                close(server_socket);
                exit(1);  
            }

            char num_packets_[MAX_LINE];
            char packet_num_[MAX_LINE];
            char size_[MAX_LINE];
            char* curr_buf = buf;

            curr_index = 0;
            while (*curr_buf != ':' && *curr_buf != '\0'){
                num_packets_[curr_index] = *curr_buf;
                ++curr_index;
                ++curr_buf;
            }

            if (*curr_buf != ':'){
                sendto(server_socket, "no", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin)); 
                continue;
            }

            num_packets_[curr_index] = '\0';
            num_packets = atoi(num_packets_);
            printf("num_packets: %d\n", num_packets);
            curr_buf = (*curr_buf == ':') ? curr_buf + 1 : curr_buf;
            
            curr_index = 0;
            while (*curr_buf != ':' && *curr_buf != '\0'){
                packet_num_[curr_index] = *curr_buf;
                ++curr_index;
                ++curr_buf;
            }
            if (*curr_buf != ':'){
                sendto(server_socket, "no", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin)); 
                continue;
            }

            packet_num_[curr_index] = '\0';
            packet_num = atoi(packet_num_);
            printf("packet_num: %d\n", packet_num);
            curr_buf = (*curr_buf == ':') ? curr_buf + 1 : curr_buf;

            curr_index = 0;
            while (*curr_buf != ':' && *curr_buf != '\0'){
                size_[curr_index] = *curr_buf;
                ++curr_index;
                ++curr_buf;
            }
            if (*curr_buf != ':'){
                sendto(server_socket, "no", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin)); 
                continue;
            }

            size_[curr_index] = '\0';
            size = atoi(size_);
            printf("size: %d\n", size);
            curr_buf = (*curr_buf == ':') ? curr_buf + 1 : curr_buf;

            curr_index = 0;
            while (*curr_buf != ':' && *curr_buf != '\0'){
                filename[curr_index] = *curr_buf;
                ++curr_index;
                ++curr_buf;
            }
            if (*curr_buf != ':'){
                sendto(server_socket, "no", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin)); 
                continue;
            }
            
            filename[curr_index] = '\0';
            printf("filename: %s\n", filename);
            curr_buf = (*curr_buf == ':') ? curr_buf + 1 : curr_buf;
            
            curr_index = 0;
            while (curr_index < size){
                payload[curr_index] = *curr_buf;
                ++curr_index;
                ++curr_buf;
            }
            payload[curr_index] = '\0';
            assert(curr_index == size);
            printf("payload: %s\n", payload);

            sendto(server_socket, "yes", strlen("yes"), 0, (struct sockaddr*) &sin, sizeof(sin));
        } while (curr_index != size);

        fp = (packet_num == 0) ? fopen(filename, "wb") : fopen(filename, "ab");

        if (fp == NULL){
            perror("filename");
            close(server_socket);
            exit(1);
        }
        
        fwrite(payload, sizeof(unsigned char), size, fp);
        fclose(fp);
    } while (packet_num < num_packets - 1);

    close(server_socket);
}