#include "message_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool message_parse_from_string(const char* raw_message, Message* message) {
    if (!raw_message || !message) return false;
    
    // Simple format: "TYPE|TIMESTAMP|USERNAME|CONTENT|METADATA"
    char* buffer = strdup(raw_message);
    char* token;
    char* saveptr;
    int field = 0;
    
    token = strtok_r(buffer, "|", &saveptr);
    while (token && field < 5) {
        switch (field) {
            case 0: // Type
                message->type = (MessageType)atoi(token);
                break;
            case 1: // Timestamp
                message->timestamp = strtoull(token, NULL, 10);
                break;
            case 2: // Username
                strncpy(message->username, token, MAX_USERNAME_LENGTH - 1);
                message->username[MAX_USERNAME_LENGTH - 1] = '\0';
                break;
            case 3: // Content
                strncpy(message->content, token, MAX_MESSAGE_LENGTH - 1);
                message->content[MAX_MESSAGE_LENGTH - 1] = '\0';
                break;
            case 4: // Metadata
                strncpy(message->metadata, token, MAX_METADATA_LENGTH - 1);
                message->metadata[MAX_METADATA_LENGTH - 1] = '\0';
                break;
        }
        token = strtok_r(NULL, "|", &saveptr);
        field++;
    }
    
    free(buffer);
    return field >= 4; // At minimum we need type, timestamp, username, content
}

int message_serialize_to_string(const Message* message, char* buffer, int buffer_size) {
    if (!message || !buffer) return -1;
    
    return snprintf(buffer, buffer_size, "%d|%llu|%s|%s|%s",
                   (int)message->type,
                   (unsigned long long)message->timestamp,
                   message->username,
                   message->content,
                   message->metadata);
}

MessageList* message_list_create(int max_messages) {
    MessageList* list = malloc(sizeof(MessageList));
    if (!list) return NULL;
    
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    list->max_messages = max_messages > 0 ? max_messages : 100; // Default limit
    
    return list;
}

void message_list_destroy(MessageList* list) {
    if (!list) return;
    
    message_list_clear(list);
    free(list);
}

bool message_list_add(MessageList* list, const Message* message) {
    if (!list || !message) return false;
    
    MessageNode* new_node = malloc(sizeof(MessageNode));
    if (!new_node) return false;
    
    new_node->message = *message;
    new_node->next = NULL;
    
    // Add to end of list
    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }
    list->tail = new_node;
    list->count++;
    
    // Remove oldest messages if we exceed limit
    while (list->count > list->max_messages && list->head) {
        MessageNode* old_head = list->head;
        list->head = list->head->next;
        if (!list->head) {
            list->tail = NULL;
        }
        free(old_head);
        list->count--;
    }
    
    return true;
}

MessageNode* message_list_get_latest(MessageList* list, int count) {
    if (!list || count <= 0) return NULL;
    
    // For simplicity, return from head (could optimize with reverse traversal)
    return list->head;
}

void message_list_clear(MessageList* list) {
    if (!list) return;
    
    while (list->head) {
        MessageNode* current = list->head;
        list->head = list->head->next;
        free(current);
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}