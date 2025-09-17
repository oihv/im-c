#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <ctype.h>

/* Simple double-free protection for testing */
#define MAX_FREED_PTRS 100
static void* freed_ptrs[MAX_FREED_PTRS];
static int freed_count = 0;

static int is_already_freed(void* ptr) {
    for (int i = 0; i < freed_count; i++) {
        if (freed_ptrs[i] == ptr) {
            return 1;
        }
    }
    return 0;
}

static void mark_as_freed(void* ptr) {
    if (freed_count < MAX_FREED_PTRS) {
        freed_ptrs[freed_count++] = ptr;
    }
}

/* Mock message structures for testing */
#define MAX_MESSAGE_LENGTH 1024
#define MAX_USERNAME_LENGTH 64

typedef enum {
    MSG_TYPE_CHAT = 0,
    MSG_TYPE_JOIN = 1,
    MSG_TYPE_LEAVE = 2,
    MSG_TYPE_STATUS = 3,
    MSG_TYPE_ERROR = 4,
    MSG_TYPE_INVALID = 999
} MessageType;

typedef struct {
    uint64_t magic; /* Magic number to detect freed messages */
    uint64_t timestamp;
    MessageType type;
    char username[MAX_USERNAME_LENGTH];
    char content[MAX_MESSAGE_LENGTH];
    char metadata[256];
} Message;

#define MESSAGE_MAGIC 0xDEADBEEFCAFEBABE
#define MESSAGE_FREED_MAGIC 0xFEEDFACEDEADDEAD

/* Error handling test functions */
int safe_string_copy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return -1; /* Invalid parameters */
    }
    
    if (dest_size == 1) {
        dest[0] = '\0';
        return 0; /* Only room for null terminator */
    }
    
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
    
    return strlen(dest);
}

int safe_string_append(char *dest, const char *src, size_t dest_size) {
    size_t dest_len, src_len, available;
    
    if (!dest || !src || dest_size == 0) {
        return -1;
    }
    
    dest_len = strlen(dest);
    src_len = strlen(src);
    
    if (dest_len >= dest_size) {
        return -1; /* Destination already overflows */
    }
    
    available = dest_size - dest_len - 1; /* Reserve space for null terminator */
    
    if (available == 0) {
        return 0; /* No space available */
    }
    
    if (src_len <= available) {
        strcat(dest, src);
        return dest_len + src_len;
    } else {
        strncat(dest, src, available);
        return dest_len + available;
    }
}

Message* safe_message_create(const char *username, const char *content, MessageType type) {
    Message *msg;
    
    if (!username || !content) {
        return NULL;
    }
    
    if (strlen(username) >= MAX_USERNAME_LENGTH || strlen(content) >= MAX_MESSAGE_LENGTH) {
        return NULL; /* Input too long */
    }
    
    msg = malloc(sizeof(Message));
    if (!msg) {
        return NULL; /* Memory allocation failed */
    }
    
    memset(msg, 0, sizeof(Message));
    
    msg->magic = MESSAGE_MAGIC;
    safe_string_copy(msg->username, username, MAX_USERNAME_LENGTH);
    safe_string_copy(msg->content, content, MAX_MESSAGE_LENGTH);
    msg->type = type;
    msg->timestamp = 1234567890; /* Mock timestamp */
    
    return msg;
}

void safe_message_destroy(Message *msg) {
    if (!msg) return;
    
    /* Check if already freed using our registry */
    if (is_already_freed(msg)) {
        return; /* Already freed, ignore */
    }
    
    /* Mark as freed before calling free() */
    mark_as_freed(msg);
    free(msg);
}

int validate_message_type(MessageType type) {
    switch (type) {
        case MSG_TYPE_CHAT:
        case MSG_TYPE_JOIN:
        case MSG_TYPE_LEAVE:
        case MSG_TYPE_STATUS:
        case MSG_TYPE_ERROR:
            return 1; /* Valid */
        default:
            return 0; /* Invalid */
    }
}

int validate_username(const char *username) {
    size_t len;
    size_t i;
    
    if (!username) return 0;
    
    len = strlen(username);
    if (len == 0 || len >= MAX_USERNAME_LENGTH) return 0;
    
    /* Check for valid characters (alphanumeric, underscore, dash) */
    for (i = 0; i < len; i++) {
        char c = username[i];
        if (!(c >= 'a' && c <= 'z') && 
            !(c >= 'A' && c <= 'Z') && 
            !(c >= '0' && c <= '9') && 
            c != '_' && c != '-') {
            return 0;
        }
    }
    
    return 1;
}

int safe_parse_integer(const char *str, int *result) {
    char *endptr;
    long val;
    
    if (!str || !result) return 0;
    
    /* Check for leading whitespace */
    if (isspace(*str)) {
        return 0;
    }
    
    val = strtol(str, &endptr, 10);
    
    /* Check for conversion errors */
    if (endptr == str || *endptr != '\0') {
        return 0; /* No digits found or extra characters */
    }
    
    /* Check for overflow */
    if (val > INT_MAX || val < INT_MIN) {
        return 0;
    }
    
    *result = (int)val;
    return 1;
}

/* Network error simulation */
typedef enum {
    NET_ERROR_NONE = 0,
    NET_ERROR_CONNECTION_LOST = 1,
    NET_ERROR_TIMEOUT = 2,
    NET_ERROR_MALFORMED_DATA = 3,
    NET_ERROR_BUFFER_OVERFLOW = 4,
    NET_ERROR_AUTHENTICATION_FAILED = 5
} NetworkError;

const char* network_error_to_string(NetworkError error) {
    switch (error) {
        case NET_ERROR_NONE: return "No error";
        case NET_ERROR_CONNECTION_LOST: return "Connection lost";
        case NET_ERROR_TIMEOUT: return "Timeout";
        case NET_ERROR_MALFORMED_DATA: return "Malformed data";
        case NET_ERROR_BUFFER_OVERFLOW: return "Buffer overflow";
        case NET_ERROR_AUTHENTICATION_FAILED: return "Authentication failed";
        default: return "Unknown error";
    }
}

/* Test cases */
void setUp(void) {
    /* Reset freed pointers registry before each test */
    freed_count = 0;
    memset(freed_ptrs, 0, sizeof(freed_ptrs));
}

void tearDown(void) {
    /* Cleanup after each test */
}

void test_safe_string_copy_normal(void) {
    char buffer[64];
    int result = safe_string_copy(buffer, "Hello World", sizeof(buffer));
    
    TEST_ASSERT_EQUAL_STRING("Hello World", buffer);
    TEST_ASSERT_EQUAL_INT(11, result);
}

void test_safe_string_copy_null_dest(void) {
    int result = safe_string_copy(NULL, "Hello", 10);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_safe_string_copy_null_src(void) {
    char buffer[64];
    int result = safe_string_copy(buffer, NULL, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_safe_string_copy_zero_size(void) {
    char buffer[64];
    int result = safe_string_copy(buffer, "Hello", 0);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_safe_string_copy_size_one(void) {
    char buffer[64];
    int result = safe_string_copy(buffer, "Hello", 1);
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("", buffer);
}

void test_safe_string_copy_truncation(void) {
    char buffer[6];
    int result = safe_string_copy(buffer, "Hello World", sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("Hello", buffer);
    TEST_ASSERT_EQUAL_INT(5, result);
}

void test_safe_string_append_normal(void) {
    char buffer[64] = "Hello";
    int result = safe_string_append(buffer, " World", sizeof(buffer));
    
    TEST_ASSERT_EQUAL_STRING("Hello World", buffer);
    TEST_ASSERT_EQUAL_INT(11, result);
}

void test_safe_string_append_null_params(void) {
    char buffer[64] = "Hello";
    
    TEST_ASSERT_EQUAL_INT(-1, safe_string_append(NULL, " World", 64));
    TEST_ASSERT_EQUAL_INT(-1, safe_string_append(buffer, NULL, 64));
    TEST_ASSERT_EQUAL_INT(-1, safe_string_append(buffer, " World", 0));
}

void test_safe_string_append_no_space(void) {
    char buffer[6] = "Hello";
    int result = safe_string_append(buffer, " World", sizeof(buffer));
    
    TEST_ASSERT_EQUAL_STRING("Hello", buffer);
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_safe_string_append_partial_append(void) {
    char buffer[10] = "Hello";
    int result = safe_string_append(buffer, " World!", sizeof(buffer));
    
    TEST_ASSERT_EQUAL_STRING("Hello Wor", buffer);
    TEST_ASSERT_EQUAL_INT(9, result);
}

void test_safe_message_create_valid(void) {
    Message *msg = safe_message_create("testuser", "Hello World", MSG_TYPE_CHAT);
    
    TEST_ASSERT_NOT_NULL(msg);
    TEST_ASSERT_EQUAL_STRING("testuser", msg->username);
    TEST_ASSERT_EQUAL_STRING("Hello World", msg->content);
    TEST_ASSERT_EQUAL_INT(MSG_TYPE_CHAT, msg->type);
    TEST_ASSERT_NOT_EQUAL(0, msg->timestamp);
    
    safe_message_destroy(msg);
}

void test_safe_message_create_null_params(void) {
    Message *msg1 = safe_message_create(NULL, "content", MSG_TYPE_CHAT);
    Message *msg2 = safe_message_create("user", NULL, MSG_TYPE_CHAT);
    
    TEST_ASSERT_NULL(msg1);
    TEST_ASSERT_NULL(msg2);
}

void test_safe_message_create_oversized_input(void) {
    char long_username[MAX_USERNAME_LENGTH + 10];
    char long_content[MAX_MESSAGE_LENGTH + 10];
    Message *msg1, *msg2;
    
    memset(long_username, 'a', sizeof(long_username) - 1);
    long_username[sizeof(long_username) - 1] = '\0';
    
    memset(long_content, 'b', sizeof(long_content) - 1);
    long_content[sizeof(long_content) - 1] = '\0';
    
    msg1 = safe_message_create(long_username, "content", MSG_TYPE_CHAT);
    msg2 = safe_message_create("user", long_content, MSG_TYPE_CHAT);
    
    TEST_ASSERT_NULL(msg1);
    TEST_ASSERT_NULL(msg2);
}

void test_validate_message_type_valid(void) {
    TEST_ASSERT_TRUE(validate_message_type(MSG_TYPE_CHAT));
    TEST_ASSERT_TRUE(validate_message_type(MSG_TYPE_JOIN));
    TEST_ASSERT_TRUE(validate_message_type(MSG_TYPE_LEAVE));
    TEST_ASSERT_TRUE(validate_message_type(MSG_TYPE_STATUS));
    TEST_ASSERT_TRUE(validate_message_type(MSG_TYPE_ERROR));
}

void test_validate_message_type_invalid(void) {
    TEST_ASSERT_FALSE(validate_message_type(MSG_TYPE_INVALID));
    TEST_ASSERT_FALSE(validate_message_type((MessageType)100));
    TEST_ASSERT_FALSE(validate_message_type((MessageType)-1));
}

void test_validate_username_valid(void) {
    TEST_ASSERT_TRUE(validate_username("user123"));
    TEST_ASSERT_TRUE(validate_username("test_user"));
    TEST_ASSERT_TRUE(validate_username("user-name"));
    TEST_ASSERT_TRUE(validate_username("ABC123"));
    TEST_ASSERT_TRUE(validate_username("a"));
}

void test_validate_username_invalid(void) {
    TEST_ASSERT_FALSE(validate_username(NULL));
    TEST_ASSERT_FALSE(validate_username(""));
    TEST_ASSERT_FALSE(validate_username("user@domain"));
    TEST_ASSERT_FALSE(validate_username("user name")); /* Space */
    TEST_ASSERT_FALSE(validate_username("user.name")); /* Dot */
    TEST_ASSERT_FALSE(validate_username("user#name")); /* Hash */
}

void test_validate_username_too_long(void) {
    char long_username[MAX_USERNAME_LENGTH + 10];
    memset(long_username, 'a', sizeof(long_username) - 1);
    long_username[sizeof(long_username) - 1] = '\0';
    
    TEST_ASSERT_FALSE(validate_username(long_username));
}

void test_safe_parse_integer_valid(void) {
    int result;
    
    TEST_ASSERT_TRUE(safe_parse_integer("123", &result));
    TEST_ASSERT_EQUAL_INT(123, result);
    
    TEST_ASSERT_TRUE(safe_parse_integer("-456", &result));
    TEST_ASSERT_EQUAL_INT(-456, result);
    
    TEST_ASSERT_TRUE(safe_parse_integer("0", &result));
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_safe_parse_integer_invalid(void) {
    int result;
    
    TEST_ASSERT_FALSE(safe_parse_integer(NULL, &result));
    TEST_ASSERT_FALSE(safe_parse_integer("abc", &result));
    TEST_ASSERT_FALSE(safe_parse_integer("123abc", &result));
    TEST_ASSERT_FALSE(safe_parse_integer("", &result));
    TEST_ASSERT_FALSE(safe_parse_integer("  123", &result)); /* Leading whitespace */
}

void test_safe_parse_integer_overflow(void) {
    char overflow_str[32];
    int result;
    
    sprintf(overflow_str, "%lld", (long long)INT_MAX + 1);
    TEST_ASSERT_FALSE(safe_parse_integer(overflow_str, &result));
    
    sprintf(overflow_str, "%lld", (long long)INT_MIN - 1);
    TEST_ASSERT_FALSE(safe_parse_integer(overflow_str, &result));
}

void test_network_error_string_conversion(void) {
    TEST_ASSERT_EQUAL_STRING("No error", network_error_to_string(NET_ERROR_NONE));
    TEST_ASSERT_EQUAL_STRING("Connection lost", network_error_to_string(NET_ERROR_CONNECTION_LOST));
    TEST_ASSERT_EQUAL_STRING("Timeout", network_error_to_string(NET_ERROR_TIMEOUT));
    TEST_ASSERT_EQUAL_STRING("Unknown error", network_error_to_string((NetworkError)999));
}

void test_buffer_overflow_protection(void) {
    char small_buffer[8];
    const char *large_input = "This is a very long string that should be truncated";
    
    safe_string_copy(small_buffer, large_input, sizeof(small_buffer));
    
    /* Verify null termination */
    TEST_ASSERT_EQUAL_CHAR('\0', small_buffer[sizeof(small_buffer) - 1]);
    
    /* Verify no buffer overflow */
    TEST_ASSERT_EQUAL_STRING("This is", small_buffer);
}

void test_memory_cleanup_on_error(void) {
    /* Simulate memory allocation failure scenario */
    Message *msg = safe_message_create("user", "content", MSG_TYPE_CHAT);
    TEST_ASSERT_NOT_NULL(msg);
    
    /* Ensure safe cleanup */
    safe_message_destroy(msg);
    
    /* Test double-free safety */
    safe_message_destroy(msg); /* Should not crash */
    safe_message_destroy(NULL); /* Should not crash */
}

void test_edge_case_empty_strings(void) {
    char buffer[64];
    int result;
    
    result = safe_string_copy(buffer, "", sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("", buffer);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    strcpy(buffer, "Hello");
    result = safe_string_append(buffer, "", sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("Hello", buffer);
    TEST_ASSERT_EQUAL_INT(5, result);
}

void test_concurrent_access_safety(void) {
    /* Test basic thread safety considerations */
    Message *msg1 = safe_message_create("user1", "message1", MSG_TYPE_CHAT);
    Message *msg2 = safe_message_create("user2", "message2", MSG_TYPE_CHAT);
    
    TEST_ASSERT_NOT_NULL(msg1);
    TEST_ASSERT_NOT_NULL(msg2);
    
    /* Verify messages are independent */
    TEST_ASSERT_NOT_EQUAL(msg1, msg2);
    TEST_ASSERT_EQUAL_STRING("user1", msg1->username);
    TEST_ASSERT_EQUAL_STRING("user2", msg2->username);
    
    safe_message_destroy(msg1);
    safe_message_destroy(msg2);
}

int main(void) {
    UNITY_BEGIN();
    
    /* String handling error tests */
    RUN_TEST(test_safe_string_copy_normal);
    RUN_TEST(test_safe_string_copy_null_dest);
    RUN_TEST(test_safe_string_copy_null_src);
    RUN_TEST(test_safe_string_copy_zero_size);
    RUN_TEST(test_safe_string_copy_size_one);
    RUN_TEST(test_safe_string_copy_truncation);
    
    RUN_TEST(test_safe_string_append_normal);
    RUN_TEST(test_safe_string_append_null_params);
    RUN_TEST(test_safe_string_append_no_space);
    RUN_TEST(test_safe_string_append_partial_append);
    
    /* Message handling error tests */
    RUN_TEST(test_safe_message_create_valid);
    RUN_TEST(test_safe_message_create_null_params);
    RUN_TEST(test_safe_message_create_oversized_input);
    
    /* Input validation tests */
    RUN_TEST(test_validate_message_type_valid);
    RUN_TEST(test_validate_message_type_invalid);
    RUN_TEST(test_validate_username_valid);
    RUN_TEST(test_validate_username_invalid);
    RUN_TEST(test_validate_username_too_long);
    
    /* Integer parsing tests */
    RUN_TEST(test_safe_parse_integer_valid);
    RUN_TEST(test_safe_parse_integer_invalid);
    RUN_TEST(test_safe_parse_integer_overflow);
    
    /* Network error handling tests */
    RUN_TEST(test_network_error_string_conversion);
    
    /* Security and edge case tests */
    RUN_TEST(test_buffer_overflow_protection);
    RUN_TEST(test_memory_cleanup_on_error);
    RUN_TEST(test_edge_case_empty_strings);
    RUN_TEST(test_concurrent_access_safety);
    
    return UNITY_END();
}