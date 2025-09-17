#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Mock structures based on protocol_lws_minimal.c */
struct msg {
    void *payload;
    size_t len;
    uint64_t timestamp;
    struct msg *next;
};

struct per_session_data__minimal {
    struct per_session_data__minimal *pss_list;
    void *wsi; /* Mocked as void* for testing */
    int last;
    int needs_history;
    struct msg *history_pos;
};

struct per_vhost_data__minimal {
    void *context; /* Mocked */
    void *vhost;   /* Mocked */
    void *protocol; /* Mocked */
    
    struct per_session_data__minimal *pss_list;
    
    struct msg amsg;
    int current;
    
    /* Message history */
    struct msg *message_history_head;
    struct msg *message_history_tail;
    int message_count;
    int max_history_messages;
};

/* Test implementations of backend functions */
static void __minimal_destroy_message(void *_msg) {
    struct msg *msg = _msg;
    if (msg && msg->payload) {
        free(msg->payload);
        msg->payload = NULL;
        msg->len = 0;
    }
}

static void __minimal_add_to_history(struct per_vhost_data__minimal *vhd, void *payload, size_t len) {
    struct msg *new_msg;
    
    if (!vhd || !payload || len == 0) return;
    
    new_msg = malloc(sizeof(struct msg));
    if (!new_msg) return;

    new_msg->payload = malloc(len + 16 + 1); /* LWS_PRE simulation + null terminator */
    if (!new_msg->payload) {
        free(new_msg);
        return;
    }

    memcpy((char *)new_msg->payload + 16, payload, len);
    ((char *)new_msg->payload + 16)[len] = '\0'; /* Null terminate string */
    new_msg->len = len;
    new_msg->timestamp = 1234567890; /* Mock timestamp */
    new_msg->next = NULL;

    /* Add to tail of history list */
    if (vhd->message_history_tail) {
        vhd->message_history_tail->next = new_msg;
    } else {
        vhd->message_history_head = new_msg;
    }
    vhd->message_history_tail = new_msg;
    vhd->message_count++;

    /* Remove old messages if we exceed limit */
    while (vhd->message_count > vhd->max_history_messages && vhd->message_history_head) {
        struct msg *old_head = vhd->message_history_head;
        vhd->message_history_head = vhd->message_history_head->next;
        if (!vhd->message_history_head) {
            vhd->message_history_tail = NULL;
        }
        free(old_head->payload);
        free(old_head);
        vhd->message_count--;
    }
}

/* Test helper functions */
static struct per_vhost_data__minimal* create_test_vhd(void) {
    struct per_vhost_data__minimal *vhd = calloc(1, sizeof(struct per_vhost_data__minimal));
    if (vhd) {
        vhd->max_history_messages = 5; /* Small limit for testing */
        vhd->message_count = 0;
        vhd->message_history_head = NULL;
        vhd->message_history_tail = NULL;
        vhd->current = 0;
    }
    return vhd;
}

static void destroy_test_vhd(struct per_vhost_data__minimal *vhd) {
    struct msg *current, *next;
    
    if (!vhd) return;
    
    /* Clean up message history */
    current = vhd->message_history_head;
    while (current) {
        next = current->next;
        if (current->payload) {
            free(current->payload);
        }
        free(current);
        current = next;
    }
    
    /* Clean up current message */
    if (vhd->amsg.payload) {
        free(vhd->amsg.payload);
    }
    
    free(vhd);
}

static struct per_session_data__minimal* create_test_pss(void) {
    struct per_session_data__minimal *pss = calloc(1, sizeof(struct per_session_data__minimal));
    if (pss) {
        pss->wsi = (void*)0x12345678; /* Mock WSI pointer */
        pss->last = 0;
        pss->needs_history = 0;
        pss->history_pos = NULL;
    }
    return pss;
}

/* Test cases */
void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

void test_minimal_destroy_message(void) {
    struct msg test_msg;
    char *test_payload = malloc(32);
    
    strcpy(test_payload, "Test message");
    test_msg.payload = test_payload;
    test_msg.len = strlen("Test message");
    
    __minimal_destroy_message(&test_msg);
    
    TEST_ASSERT_NULL(test_msg.payload);
    TEST_ASSERT_EQUAL_INT(0, test_msg.len);
}

void test_minimal_add_to_history_single_message(void) {
    struct per_vhost_data__minimal *vhd = create_test_vhd();
    const char *test_message = "Hello World";
    
    __minimal_add_to_history(vhd, (void*)test_message, strlen(test_message));
    
    TEST_ASSERT_EQUAL_INT(1, vhd->message_count);
    TEST_ASSERT_NOT_NULL(vhd->message_history_head);
    TEST_ASSERT_EQUAL_PTR(vhd->message_history_head, vhd->message_history_tail);
    TEST_ASSERT_EQUAL_INT(strlen(test_message), vhd->message_history_head->len);
    
    /* Check message content (offset by LWS_PRE simulation) */
    char *stored_message = (char*)vhd->message_history_head->payload + 16;
    TEST_ASSERT_EQUAL_STRING(test_message, stored_message);
    
    destroy_test_vhd(vhd);
}

void test_minimal_add_to_history_multiple_messages(void) {
    struct per_vhost_data__minimal *vhd = create_test_vhd();
    const char *messages[] = {"Message 1", "Message 2", "Message 3"};
    int i;
    struct msg *current;
    
    for (i = 0; i < 3; i++) {
        __minimal_add_to_history(vhd, (void*)messages[i], strlen(messages[i]));
    }
    
    TEST_ASSERT_EQUAL_INT(3, vhd->message_count);
    TEST_ASSERT_NOT_NULL(vhd->message_history_head);
    TEST_ASSERT_NOT_NULL(vhd->message_history_tail);
    TEST_ASSERT_NOT_EQUAL(vhd->message_history_head, vhd->message_history_tail);
    
    /* Verify message order */
    current = vhd->message_history_head;
    for (i = 0; i < 3; i++) {
        TEST_ASSERT_NOT_NULL(current);
        char *stored_message = (char*)current->payload + 16;
        TEST_ASSERT_EQUAL_STRING(messages[i], stored_message);
        current = current->next;
    }
    
    destroy_test_vhd(vhd);
}

void test_minimal_add_to_history_overflow(void) {
    struct per_vhost_data__minimal *vhd = create_test_vhd();
    const char *messages[] = {"Msg1", "Msg2", "Msg3", "Msg4", "Msg5", "Msg6", "Msg7"};
    int i;
    struct msg *current;
    
    /* Add more messages than max_history_messages (5) */
    for (i = 0; i < 7; i++) {
        __minimal_add_to_history(vhd, (void*)messages[i], strlen(messages[i]));
    }
    
    /* Should only keep the last 5 messages */
    TEST_ASSERT_EQUAL_INT(5, vhd->message_count);
    
    /* Verify we have the last 5 messages (Msg3, Msg4, Msg5, Msg6, Msg7) */
    current = vhd->message_history_head;
    for (i = 2; i < 7; i++) { /* Start from index 2 (Msg3) */
        TEST_ASSERT_NOT_NULL(current);
        char *stored_message = (char*)current->payload + 16;
        TEST_ASSERT_EQUAL_STRING(messages[i], stored_message);
        current = current->next;
    }
    
    destroy_test_vhd(vhd);
}

void test_minimal_add_to_history_null_inputs(void) {
    struct per_vhost_data__minimal *vhd = create_test_vhd();
    
    /* Test with NULL payload */
    __minimal_add_to_history(vhd, NULL, 10);
    TEST_ASSERT_EQUAL_INT(0, vhd->message_count);
    
    /* Test with zero length */
    __minimal_add_to_history(vhd, "test", 0);
    TEST_ASSERT_EQUAL_INT(0, vhd->message_count);
    
    /* Test with NULL vhd */
    __minimal_add_to_history(NULL, "test", 4);
    /* Should not crash */
    
    destroy_test_vhd(vhd);
}

void test_session_initialization(void) {
    struct per_session_data__minimal *pss = create_test_pss();
    
    TEST_ASSERT_NOT_NULL(pss);
    TEST_ASSERT_EQUAL_INT(0, pss->last);
    TEST_ASSERT_EQUAL_INT(0, pss->needs_history);
    TEST_ASSERT_NULL(pss->history_pos);
    TEST_ASSERT_NOT_NULL(pss->wsi);
    
    free(pss);
}

void test_vhost_data_initialization(void) {
    struct per_vhost_data__minimal *vhd = create_test_vhd();
    
    TEST_ASSERT_NOT_NULL(vhd);
    TEST_ASSERT_EQUAL_INT(5, vhd->max_history_messages);
    TEST_ASSERT_EQUAL_INT(0, vhd->message_count);
    TEST_ASSERT_NULL(vhd->message_history_head);
    TEST_ASSERT_NULL(vhd->message_history_tail);
    TEST_ASSERT_EQUAL_INT(0, vhd->current);
    TEST_ASSERT_NULL(vhd->amsg.payload);
    
    destroy_test_vhd(vhd);
}

void test_message_structure_integrity(void) {
    struct msg test_msg;
    const char *test_payload = "Test message for integrity check";
    
    test_msg.payload = malloc(strlen(test_payload) + 1);
    strcpy(test_msg.payload, test_payload);
    test_msg.len = strlen(test_payload);
    test_msg.timestamp = 1234567890;
    test_msg.next = NULL;
    
    TEST_ASSERT_NOT_NULL(test_msg.payload);
    TEST_ASSERT_EQUAL_INT(strlen(test_payload), test_msg.len);
    TEST_ASSERT_EQUAL_STRING(test_payload, (char*)test_msg.payload);
    TEST_ASSERT_EQUAL_UINT64(1234567890, test_msg.timestamp);
    TEST_ASSERT_NULL(test_msg.next);
    
    free(test_msg.payload);
}

void test_linked_list_operations(void) {
    struct per_vhost_data__minimal *vhd = create_test_vhd();
    const char *first_msg = "First";
    const char *second_msg = "Second";
    
    /* Add first message */
    __minimal_add_to_history(vhd, (void*)first_msg, strlen(first_msg));
    TEST_ASSERT_EQUAL_PTR(vhd->message_history_head, vhd->message_history_tail);
    TEST_ASSERT_NULL(vhd->message_history_head->next);
    
    /* Add second message */
    __minimal_add_to_history(vhd, (void*)second_msg, strlen(second_msg));
    TEST_ASSERT_NOT_EQUAL(vhd->message_history_head, vhd->message_history_tail);
    TEST_ASSERT_EQUAL_PTR(vhd->message_history_head->next, vhd->message_history_tail);
    TEST_ASSERT_NULL(vhd->message_history_tail->next);
    
    destroy_test_vhd(vhd);
}

void test_memory_management(void) {
    struct per_vhost_data__minimal *vhd = create_test_vhd();
    const char *test_message = "Memory test message";
    void *original_payload;
    
    /* Add message and verify payload allocation */
    __minimal_add_to_history(vhd, (void*)test_message, strlen(test_message));
    TEST_ASSERT_NOT_NULL(vhd->message_history_head->payload);
    
    original_payload = vhd->message_history_head->payload;
    
    /* Destroy message and verify cleanup */
    __minimal_destroy_message(vhd->message_history_head);
    TEST_ASSERT_NULL(vhd->message_history_head->payload);
    TEST_ASSERT_EQUAL_INT(0, vhd->message_history_head->len);
    
    /* Cleanup */
    free(vhd->message_history_head);
    vhd->message_history_head = NULL;
    vhd->message_history_tail = NULL;
    vhd->message_count = 0;
    
    destroy_test_vhd(vhd);
}

int main(void) {
    UNITY_BEGIN();
    
    /* Message management tests */
    RUN_TEST(test_minimal_destroy_message);
    RUN_TEST(test_minimal_add_to_history_single_message);
    RUN_TEST(test_minimal_add_to_history_multiple_messages);
    RUN_TEST(test_minimal_add_to_history_overflow);
    RUN_TEST(test_minimal_add_to_history_null_inputs);
    
    /* Data structure tests */
    RUN_TEST(test_session_initialization);
    RUN_TEST(test_vhost_data_initialization);
    RUN_TEST(test_message_structure_integrity);
    
    /* Internal logic tests */
    RUN_TEST(test_linked_list_operations);
    RUN_TEST(test_memory_management);
    
    return UNITY_END();
}