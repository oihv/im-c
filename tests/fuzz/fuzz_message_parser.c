#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

/* Include message types for fuzzing */
#include "message_types.h"

/* AFL-style fuzzing harness for message parser */

/* Mock message parsing function for fuzzing */
typedef enum {
    FUZZ_MSG_TYPE_CHAT = 0,
    FUZZ_MSG_TYPE_JOIN = 1,
    FUZZ_MSG_TYPE_LEAVE = 2,
    FUZZ_MSG_TYPE_STATUS = 3,
    FUZZ_MSG_TYPE_ERROR = 4
} FuzzMessageType;

typedef struct {
    uint64_t timestamp;
    FuzzMessageType type;
    char username[64];
    char content[1024];
    char metadata[256];
} FuzzMessage;

/* Safe parsing function that handles malformed input */
int fuzz_message_parse_from_string(const char *input, FuzzMessage *message) {
    char buffer[2048];
    char *token;
    char *saveptr;
    int field_count = 0;
    
    if (!input || !message || strlen(input) >= sizeof(buffer)) {
        return 0; /* Invalid input */
    }
    
    /* Initialize message */
    memset(message, 0, sizeof(FuzzMessage));
    
    /* Copy input to working buffer */
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    /* Parse JSON-like format: {"timestamp":123,"type":0,"username":"user","content":"text"} */
    token = strtok_r(buffer, ":{},\"", &saveptr);
    
    while (token && field_count < 8) { /* Limit iterations */
        if (strcmp(token, "timestamp") == 0) {
            token = strtok_r(NULL, ":{},\"", &saveptr);
            if (token) {
                message->timestamp = strtoull(token, NULL, 10);
                field_count++;
            }
        } else if (strcmp(token, "type") == 0) {
            token = strtok_r(NULL, ":{},\"", &saveptr);
            if (token) {
                int type_val = atoi(token);
                if (type_val >= 0 && type_val <= 4) {
                    message->type = (FuzzMessageType)type_val;
                    field_count++;
                }
            }
        } else if (strcmp(token, "username") == 0) {
            token = strtok_r(NULL, ":{},\"", &saveptr);
            if (token) {
                strncpy(message->username, token, sizeof(message->username) - 1);
                field_count++;
            }
        } else if (strcmp(token, "content") == 0) {
            token = strtok_r(NULL, ":{},\"", &saveptr);
            if (token) {
                strncpy(message->content, token, sizeof(message->content) - 1);
                field_count++;
            }
        } else if (strcmp(token, "metadata") == 0) {
            token = strtok_r(NULL, ":{},\"", &saveptr);
            if (token) {
                strncpy(message->metadata, token, sizeof(message->metadata) - 1);
                field_count++;
            }
        }
        
        token = strtok_r(NULL, ":{},\"", &saveptr);
    }
    
    /* Require at least username and content */
    return (strlen(message->username) > 0 && strlen(message->content) > 0);
}

/* Test various parsing edge cases */
void fuzz_test_edge_cases(const char *input, size_t len) {
    FuzzMessage message;
    
    /* Test with null termination variations */
    char *test_input = malloc(len + 1);
    if (!test_input) return;
    
    memcpy(test_input, input, len);
    test_input[len] = '\0';
    
    /* Attempt to parse */
    int result = fuzz_message_parse_from_string(test_input, &message);
    
    /* Additional validation if parsing succeeded */
    if (result) {
        /* Verify no buffer overflows occurred */
        if (strlen(message.username) >= 64 || 
            strlen(message.content) >= 1024 ||
            strlen(message.metadata) >= 256) {
            /* This would indicate a buffer overflow */
            abort();
        }
        
        /* Verify timestamp is reasonable */
        if (message.timestamp > 0xFFFFFFFFFFFFFFFF) {
            abort();
        }
        
        /* Verify type is valid */
        if (message.type < 0 || message.type > 4) {
            abort();
        }
    }
    
    free(test_input);
}

/* Test string manipulation functions */
void fuzz_test_string_operations(const char *input, size_t len) {
    char buffer1[256];
    char buffer2[256];
    
    if (len > 0 && len < 200) {
        /* Test safe string copy */
        memset(buffer1, 0, sizeof(buffer1));
        strncpy(buffer1, input, len);
        buffer1[sizeof(buffer1) - 1] = '\0';
        
        /* Test string concatenation */
        if (strlen(buffer1) < 100) {
            strncat(buffer1, " SUFFIX", sizeof(buffer1) - strlen(buffer1) - 1);
        }
        
        /* Test string comparison */
        memset(buffer2, 0, sizeof(buffer2));
        strcpy(buffer2, "TEST");
        
        int cmp_result = strcmp(buffer1, buffer2);
        (void)cmp_result; /* Use result to prevent optimization */
    }
}

/* Main fuzzing entry point */
int main(int argc, char **argv) {
    char input_buffer[4096];
    size_t len;
    
    /* Read input from stdin (AFL-style) or file */
    if (argc > 1) {
        /* File input mode */
        FILE *fp = fopen(argv[1], "rb");
        if (!fp) {
            perror("fopen");
            return 1;
        }
        
        len = fread(input_buffer, 1, sizeof(input_buffer) - 1, fp);
        fclose(fp);
    } else {
        /* Stdin mode (for AFL) */
        len = read(STDIN_FILENO, input_buffer, sizeof(input_buffer) - 1);
        if (len <= 0) {
            return 1;
        }
    }
    
    input_buffer[len] = '\0';
    
    /* Run fuzz tests */
    fuzz_test_edge_cases(input_buffer, len);
    fuzz_test_string_operations(input_buffer, len);
    
    /* Test with various null byte positions */
    for (size_t i = 0; i < len && i < 10; i++) {
        char *modified_input = malloc(len + 1);
        if (modified_input) {
            memcpy(modified_input, input_buffer, len);
            modified_input[i] = '\0'; /* Insert null byte at position i */
            
            fuzz_test_edge_cases(modified_input, i);
            
            free(modified_input);
        }
    }
    
    /* Test with repeated characters */
    char repeated_input[100];
    if (len > 0) {
        memset(repeated_input, input_buffer[0], sizeof(repeated_input) - 1);
        repeated_input[sizeof(repeated_input) - 1] = '\0';
        fuzz_test_edge_cases(repeated_input, sizeof(repeated_input) - 1);
    }
    
    return 0;
}

/* Standalone test cases for development */
#ifdef FUZZ_STANDALONE_TEST
int test_fuzz_basic_cases(void) {
    FuzzMessage message;
    int result;
    
    /* Test 1: Valid message */
    result = fuzz_message_parse_from_string(
        "{\"timestamp\":1234567890,\"type\":0,\"username\":\"testuser\",\"content\":\"hello world\"}",
        &message
    );
    if (!result || strcmp(message.username, "testuser") != 0) {
        printf("FAIL: Basic valid message test\n");
        return 1;
    }
    
    /* Test 2: Malformed JSON */
    result = fuzz_message_parse_from_string(
        "{\"timestamp\":1234567890,\"type\":0,\"username\":\"testuser\",\"content\":\"hello",
        &message
    );
    /* Should handle gracefully */
    
    /* Test 3: Empty string */
    result = fuzz_message_parse_from_string("", &message);
    if (result != 0) {
        printf("FAIL: Empty string should return 0\n");
        return 1;
    }
    
    /* Test 4: NULL input */
    result = fuzz_message_parse_from_string(NULL, &message);
    if (result != 0) {
        printf("FAIL: NULL input should return 0\n");
        return 1;
    }
    
    /* Test 5: Very long input */
    char long_input[3000];
    memset(long_input, 'A', sizeof(long_input) - 1);
    long_input[sizeof(long_input) - 1] = '\0';
    result = fuzz_message_parse_from_string(long_input, &message);
    /* Should handle gracefully without crash */
    
    printf("Basic fuzz tests completed successfully\n");
    return 0;
}

int main(void) {
    return test_fuzz_basic_cases();
}
#endif