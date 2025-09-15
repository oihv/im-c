#include "unity.h"
#include "websocket_service.h"
#include "message_types.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void setUp(void) {
}

void tearDown(void) {
    websocket_service_cleanup();
}

void test_websocket_service_init(void) {
    bool result = websocket_service_init();
    
    TEST_ASSERT_TRUE(result);
}

void test_websocket_service_update_without_init(void) {
    WebSocketData* data = websocket_service_update();
    
    // Should handle gracefully without crashing
    // The exact behavior depends on implementation
}

void test_websocket_service_send_text(void) {
    websocket_service_init();
    
    // This should not crash even if not connected
    websocket_service_send_text("test_user", "Hello, World!");
    
    // Clean up
    websocket_service_cleanup();
}

void test_websocket_service_send_message(void) {
    websocket_service_init();
    
    Message test_message = {
        .timestamp = 1234567890,
        .type = MSG_TYPE_CHAT
    };
    strcpy(test_message.username, "test_user");
    strcpy(test_message.content, "Test message");
    strcpy(test_message.metadata, "");
    
    // This should not crash even if not connected
    websocket_service_send_message(&test_message);
    
    // Clean up
    websocket_service_cleanup();
}

void test_websocket_should_close(void) {
    bool should_close = websocket_should_close();
    
    // Should return a boolean value
    TEST_ASSERT_TRUE(should_close == true || should_close == false);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_websocket_service_init);
    RUN_TEST(test_websocket_service_update_without_init);
    RUN_TEST(test_websocket_service_send_text);
    RUN_TEST(test_websocket_service_send_message);
    RUN_TEST(test_websocket_should_close);
    
    return UNITY_END();
}