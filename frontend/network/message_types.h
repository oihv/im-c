#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#define MAX_MESSAGE_LENGTH 512
#define MAX_USERNAME_LENGTH 64
#define MAX_METADATA_LENGTH 128

typedef enum {
    MSG_TYPE_CHAT = 0,
    MSG_TYPE_JOIN = 1,
    MSG_TYPE_LEAVE = 2,
    MSG_TYPE_STATUS = 3,
    MSG_TYPE_HISTORY_REQUEST = 4,
    MSG_TYPE_HISTORY_RESPONSE = 5
} MessageType;

typedef struct {
    uint64_t timestamp;
    char username[MAX_USERNAME_LENGTH];
    MessageType type;
    char content[MAX_MESSAGE_LENGTH];
    char metadata[MAX_METADATA_LENGTH];
} Message;

typedef struct MessageNode {
    Message message;
    struct MessageNode* next;
} MessageNode;

typedef struct {
    MessageNode* head;
    MessageNode* tail;
    int count;
    int max_messages;
} MessageList;

// Message parsing functions
bool message_parse_from_string(const char* raw_message, Message* message);
int message_serialize_to_string(const Message* message, char* buffer, int buffer_size);

// Message list functions
MessageList* message_list_create(int max_messages);
void message_list_destroy(MessageList* list);
bool message_list_add(MessageList* list, const Message* message);
MessageNode* message_list_get_latest(MessageList* list, int count);
void message_list_clear(MessageList* list);

#endif