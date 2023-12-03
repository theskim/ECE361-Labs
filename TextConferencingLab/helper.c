#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "message.h"

// Helper function to clear a given char* buffer
void clear_buffer(char* buf){
    for (int i = 0; i < MAX_LINE; ++i)
        buf[i] = '\0';
}

// Helper function to free a given pointer and set it to NULL
void smart_free(void** ptr){
    if (*ptr != NULL){
        free(*ptr);
        *ptr = NULL;
    }
}

// Helper function to display a message for debugging purposes
void print_message(Message message){
    printf("Message:\n");
    printf("\tType: %d\n", message.type);
    printf("\tSize: %d\n", message.size);
    printf("\tSource: %s\n", message.source);
    printf("\tData: %s\n", message.data);
}

// Given a message, convert it to a string (NOTE THAT THIS FUNCTION USES MALLOC, USER MUST FREE THE RETURNED STRING)
void get_string_from_message(char* message_string, Message message){
    // Format: type:size:source:data 
    strcpy(message_string, "");
    sprintf(message_string + strlen(message_string), "%d", (int)message.type);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%d", message.size);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%s", message.source);
    strcat(message_string, ":");
    sprintf(message_string + strlen(message_string), "%s", message.data);
}

// Given a string, convert it to a message (message is passed by reference)
void get_message_from_string(char* string_received, Message* message){
    char* splitted_val = strtok(string_received, ":");

    // Format: type:size:source:data -> seperate it by : and store it in message
    for (int i = 0; i <= 3; ++i){
        if (splitted_val == NULL){ // if token is null, something is wrong
            if (i == 0)
                message->type = INVALID;
            else if (i == 1)
                message->size = INVALID;
            else if (i == 2)
                continue;
            else if (i == 3)
                continue;
        }
        else {
            if (i == 0)
                message->type = (Type)atoi(splitted_val);
            else if (i == 1)
                message->size = atoi(splitted_val);
            else if (i == 2)
                strcpy((char *)message->source, splitted_val);
            else if (i == 3)
                strcpy((char *)message->data, splitted_val);
        }
        splitted_val = strtok(NULL, ":");
    }
}
