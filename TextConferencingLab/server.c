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
#include <pthread.h>
#include <stdbool.h>
#include "message.h"
#include "helper.h"

// hard-coded users:
unsigned char IDs[][MAX_NAME] = {"Steve", "Jill", "Grace", "Joe"};
unsigned char passwords[][MAX_PASSWORD] = {"Steve1", "Jill2", "Grace3", "Joe4"};

Client* head = NULL;
Session* session_head = NULL;

// Helper function to print all clients for debugging purposes (traversing linked list)
void print_clients(){
    Client* iterator = head;
    printf("Current Clients:\n");
    int i = 0;
    while (iterator != NULL){
        printf("\tClient %d: ID of %s, IP of %s, Port of %d, Session ID of %d\n", i, iterator->ID, iterator->IP, iterator->port, iterator->session_ID);
        iterator = iterator->next;
        ++i;
    }
    printf("\n");
}

// Helper function to register a client into a linked list
int register_client(unsigned char* ID, char* IP, unsigned int port, unsigned char* password){
    // int flag = 0;
    // for (int i = 0; i < 4; i++)
    //     if (!strcmp((char *)IDs[i], (char *)ID) && !strcmp((char *)passwords[i], (char *)password))
    //         flag = 1;

    // if (!flag){
    //     printf("Invalid credentials");
    //     return -1;
    // }
    printf("Registering client ID = %s \n", ID);
    print_clients();

    // Essentially inserting a value into a linked list 

    Client* newNode = malloc(sizeof(Client));
    newNode->session_ID = -1;
    strcpy((char *)newNode->ID, (char *)ID);
    strcpy((char *)newNode->IP, (char *)IP);
    strcpy((char *)newNode->password, (char *)password);
    newNode->port = port;
    newNode->next = NULL;

    if (head == NULL){
        head = newNode;
        return 1;
    }

    Client* traverser = head;
    while (traverser->next != NULL && strcmp((char *)traverser->ID,(char *)ID))
        traverser = traverser->next;
    
    if (!strcmp((char *)traverser->ID, (char *)ID)){
        free(newNode);
        printf("User already registered\n");
        return -1;
    }
    
    traverser->next = newNode;
    return 1;
}
 
// Helper function to remove a client from a linked list given their ID
int remove_client(unsigned char* ID){
    printf("Removing client ID: %s \n", ID);
    if (head != NULL){
        if (head->next == NULL && !strcmp((char *)head->ID, (char *)ID)){
            smart_free((void**)&head); // free the memory to avoid memory leak
            return 1;
        }
        if (head->next == NULL && strcmp((char *)head->ID, (char *)ID)){
            printf("ID %s not found\n", ID);
            return -1;
        }
    } else {
        printf("List already empty\n");
        return -1; 
    }
    
    if (!strcmp((char *)head->ID, (char *)ID)){
        Client* temp = head;
        head = head->next;
        smart_free((void** )&temp); // free the memory to avoid memory leak
        return 1;
    }
    
    Client *traverser = head->next;
    Client *prev_node = head;

    while (traverser->next != NULL && strcmp((char *)traverser->ID, (char *)ID)){
        prev_node = prev_node->next;
        traverser = traverser->next;
    }
    if (strcmp((char *)traverser->ID,(char *)ID)){
        printf("ID %s not found\n", ID);
        return -1;
    }
    
    prev_node->next = traverser->next;
    smart_free((void** )&traverser); 

    return 1;
}

// Thread function to handle client requests
void* client_receiver(void* args){
    thread_args arguments = *(thread_args*)args;
    int client_socket = arguments.socket;
    char* client_IP = arguments.IP;
    int client_port = arguments.port;
    smart_free((void** )&args); 

    char string_received[MAX_DATA];
    int read_size;

    // Read from client
    if ((read_size = read(client_socket, string_received, MAX_DATA)) < 0){
        perror("read");
        exit(1);
    }
    string_received[read_size] = '\0'; // safety in case 

    Message* message = malloc(sizeof(Message));
    *message = (Message){0, 0, "", ""}; // empty message
    get_message_from_string(string_received, message); // get message from string
    print_message(*message); // print message

    // if valid, send ACK
    if (message->type == LOGIN){
        Message new_message;
        
        // Check if NACK of login
        if (message->size == 0 || !message->source || !message->data){ // check if size is 0 (loss)
            new_message.type = LO_NAK;
            new_message.size = strlen("Empty fields (potential packet loss)");
            strcpy((char *)new_message.source, "server");
            strcpy((char *)new_message.data, "Empty fields (potential packet loss)");
        } 
        else if (register_client(message->source, client_IP, client_port, message->data) == -1){ // check if valid login
            new_message.type = LO_NAK;
            new_message.size = strlen("Registration failed, User already exists");
            strcpy((char *)new_message.source, "server");
            strcpy((char *)new_message.data, "Registration failed, User already exists");
        }
        else {  
            new_message.type = LO_ACK;
            new_message.size = strlen("Successful");
            strcpy((char *)new_message.source, "server");
            strcpy((char *)new_message.data, "Successful");
        }

        char message_string[MAX_LINE];
        get_string_from_message(message_string, new_message);
        if (write(client_socket, message_string, strlen(message_string)) < 0){
            smart_free((void**)&message); 
            perror("write");
            exit(1);
        }
    } 
    else if (message->type == EXIT) // exit
        remove_client(message->source);
    else if (message->type == JOIN){ // join
        Message* new_message = malloc(sizeof(Message));

        // TODO (check if session exists, if doesn't send NAK)
        bool session_exist = true; // modify this
        if (session_exist){
            new_message->type = JN_ACK; 
        } else {
            new_message->type = JN_NAK;
        }
    }
    else if (message->type == LEAVE_SESS){ // leave
        // TODO (remove client from session)
    }
    else if (message->type == NEW_SESS){ // new
        Message* new_message = malloc(sizeof(Message));
        
        new_message->type = NS_ACK; // always ACK for new session
        // TODO: create a new session (possibly in a list)
    }
    else if (message->type == MESSAGE){ // message
        // TODO: need to send message to all clients in session
    }
    else if (message->type == QUERY){ // query (list)
        Message* new_message = malloc(sizeof(Message));
        
        new_message->type = QU_ACK; // always ACK for query
        strcpy((char *)new_message->source, "server");

        Client* iterator = head;
        while (iterator != NULL){ // format: ID|session_ID|ID|session_ID....
            sprintf((char *)new_message->data + strlen((char *)new_message->data), "%s", iterator->ID);
            strcat((char *)new_message->data, "|");
            sprintf((char *)new_message->data + strlen((char *)new_message->data), "%d", iterator->session_ID);
            strcat((char *)new_message->data, "|");
            iterator = iterator->next;
        }
        new_message->size = strlen((char *)new_message->data);
        print_message(*new_message);
        char message_string[MAX_LINE];
        get_string_from_message(message_string, *new_message);
        smart_free((void**)&new_message); // free the memory to avoid memory leak
        
        //printf("%s\n", message_string);
        if (write(client_socket, message_string, strlen(message_string)) < 0){
            smart_free((void**)&message); 
            perror("write");
            exit(1);
        }
    }

    smart_free((void**)&message); 
    print_clients();

    return 0;
}

int main(int argc, char *argv[]){
    sockaddr_in sin;
    socklen_t addr_len = sizeof(sin);
    int server_socket;
    char* str_end;
    char buf[MAX_DATA];

    //use me!
    //https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

    // ./server <port>
    if (argc != 2){
        perror("Should have two arguments");
        exit(1);
    }

    // Create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        close(server_socket);
        exit(1);
    }

    bzero(&sin, sizeof(sin)); // fills a buffer with zero bytes
    sin.sin_family = AF_INET; // IPv4
    sin.sin_addr.s_addr = INADDR_ANY; // update address (0.0.0.0)
    long port = strtol(argv[1], &str_end, 10);

    if (errno == ERANGE || str_end == argv[1] || *str_end != '\0'){ // nothing received or err
        perror("invalid port");
        close(server_socket);
        exit(1);
    }
    sin.sin_port = htons(port); // host <-> network byte order
    
    if ((bind(server_socket, (sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("bind");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, SAFE_BACKLOG) < 0){
        perror("listen");
        close(server_socket);
        exit(1);  
    }

    int client_socket;
    int* new_socket;
    while (true){ // accept connections continuously
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        if ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) < 0){
            perror("accept");
            close(server_socket);
            exit(1);
        }

        char* client_IP = inet_ntoa(client_addr.sin_addr); // convert IP to string
        int client_port = ntohs(client_addr.sin_port); // convert port to int
        printf("Connection Accepted from %s:%d\n", client_IP, client_port); // print client info

        pthread_t new_thread;
        thread_args* args = malloc(sizeof(thread_args)); // allocate memory for thread args (to access them from thread)
        args->socket = client_socket;
        args->IP = client_IP;
        args->port = client_port;
        
        // Create a new thread and join it afterwards
        if (pthread_create(&new_thread, NULL, client_receiver, (void*)args) < 0){
            free(args);
            perror("pthread_create");
            exit(1);
        }

        pthread_join(new_thread, NULL); // join this thread to receive client req
    }
    
    printf("Closing server socket...\n");
    close(server_socket); 
    
    return 0;
}