#include "unity.h"
#include "message_types.h"
#include <string.h>
#include <stdlib.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_message_parse_from_string_valid_chat_message(void) {
    const char* pipe_delimited_str = "0|1234567890|test_user|Hello, World!|";
    Message message;
    
    bool result = message_parse_from_string(pipe_delimited_str, &message);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT64(1234567890, message.timestamp);
    TEST_ASSERT_EQUAL_STRING("test_user", message.username);
    TEST_ASSERT_EQUAL_INT(MSG_TYPE_CHAT, message.type);
    TEST_ASSERT_EQUAL_STRING("Hello, World!", message.content);
}

void test_message_parse_from_string_invalid_format(void) {
    const char* invalid_format = "0|1234567890|test_user"; // Missing required fields
    Message message;
    
    bool result = message_parse_from_string(invalid_format, &message);
    
    TEST_ASSERT_FALSE(result);
}

void test_message_serialize_to_string(void) {
    Message message = {
        .timestamp = 1234567890,
        .type = MSG_TYPE_CHAT
    };
    strcpy(message.username, "test_user");
    strcpy(message.content, "Hello, World!");
    strcpy(message.metadata, "");
    
    char buffer[1024];
    int result = message_serialize_to_string(&message, buffer, sizeof(buffer));
    
    TEST_ASSERT_GREATER_THAN(0, result);
    TEST_ASSERT_TRUE(strstr(buffer, "test_user") != NULL);
    TEST_ASSERT_TRUE(strstr(buffer, "Hello, World!") != NULL);
    TEST_ASSERT_TRUE(strstr(buffer, "1234567890") != NULL);
}

void test_message_list_create_and_destroy(void) {
    MessageList* list = message_list_create(10);
    
    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_EQUAL_INT(0, list->count);
    TEST_ASSERT_EQUAL_INT(10, list->max_messages);
    TEST_ASSERT_NULL(list->head);
    TEST_ASSERT_NULL(list->tail);
    
    message_list_destroy(list);
}

void test_message_list_add_single_message(void) {
    MessageList* list = message_list_create(10);
    Message message = {
        .timestamp = 1234567890,
        .type = MSG_TYPE_CHAT
    };
    strcpy(message.username, "test_user");
    strcpy(message.content, "Test message");
    strcpy(message.metadata, "");
    
    bool result = message_list_add(list, &message);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(1, list->count);
    TEST_ASSERT_NOT_NULL(list->head);
    TEST_ASSERT_NOT_NULL(list->tail);
    TEST_ASSERT_EQUAL_PTR(list->head, list->tail);
    TEST_ASSERT_EQUAL_STRING("test_user", list->head->message.username);
    
    message_list_destroy(list);
}

void test_message_list_add_multiple_messages(void) {
    MessageList* list = message_list_create(10);
    
    for (int i = 0; i < 5; i++) {
        Message message = {
            .timestamp = 1234567890 + i,
            .type = MSG_TYPE_CHAT
        };
        sprintf(message.username, "user_%d", i);
        sprintf(message.content, "message_%d", i);
        strcpy(message.metadata, "");
        
        bool result = message_list_add(list, &message);
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_EQUAL_INT(5, list->count);
    TEST_ASSERT_NOT_NULL(list->head);
    TEST_ASSERT_NOT_NULL(list->tail);
    
    message_list_destroy(list);
}

void test_message_list_overflow_protection(void) {
    MessageList* list = message_list_create(3);
    
    for (int i = 0; i < 5; i++) {
        Message message = {
            .timestamp = 1234567890 + i,
            .type = MSG_TYPE_CHAT
        };
        sprintf(message.username, "user_%d", i);
        sprintf(message.content, "message_%d", i);
        strcpy(message.metadata, "");
        
        bool result = message_list_add(list, &message);
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_EQUAL_INT(3, list->count);
    
    message_list_destroy(list);
}

void test_message_list_clear(void) {
    MessageList* list = message_list_create(10);
    
    Message message = {
        .timestamp = 1234567890,
        .type = MSG_TYPE_CHAT
    };
    strcpy(message.username, "test_user");
    strcpy(message.content, "Test message");
    strcpy(message.metadata, "");
    
    message_list_add(list, &message);
    TEST_ASSERT_EQUAL_INT(1, list->count);
    
    message_list_clear(list);
    TEST_ASSERT_EQUAL_INT(0, list->count);
    TEST_ASSERT_NULL(list->head);
    TEST_ASSERT_NULL(list->tail);
    
    message_list_destroy(list);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_message_parse_from_string_valid_chat_message);
    RUN_TEST(test_message_parse_from_string_invalid_format);
    RUN_TEST(test_message_serialize_to_string);
    RUN_TEST(test_message_list_create_and_destroy);
    RUN_TEST(test_message_list_add_single_message);
    RUN_TEST(test_message_list_add_multiple_messages);
    RUN_TEST(test_message_list_overflow_protection);
    RUN_TEST(test_message_list_clear);
    
    return UNITY_END();
}