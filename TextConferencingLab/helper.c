#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "message.h"

void print_message(Message message){
    printf("Type: %d\n", message.type);
    printf("Size: %d\n", message.size);
    printf("Source: %s\n", message.source);
    printf("Data: %s\n", message.data);
}

char* get_string_from_message(Message message){
    // Format: type:size:source:data 
    char* message_string = malloc(MAX_LINE);
    strcpy(message_string, "");
    sprintf(message_string + strlen(message_string), "%d", (int)message.type);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%d", message.size);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%s", message.source);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%s", message.data);
    
    return message_string;
}


void get_message_from_string(char* string_received, Message* message){
    char* token = strtok(string_received, ":");

    // Format: type:size:source:data -> seperate it by : and store it in message
    for (int i = 0; i <= 3; ++i){
        if (token == NULL){ // if token is null, something is wrong
            if (i == 0)
                message->type = INVALID;
            else if (i == 1)
                message->size = INVALID;
            else if (i == 2)
                strcpy((char *)message->source, token);
            else if (i == 3)
                strcpy((char *)message->data, token);
        }
        else {
            if (i == 0)
                message->type = (Type)atoi(token);
            else if (i == 1)
                message->size = atoi(token);
            else if (i == 2)
                strcpy((char *)message->source, token);
            else if (i == 3)
                strcpy((char *)message->data, token);
        }
        token = strtok(NULL, ":");
    }
}
