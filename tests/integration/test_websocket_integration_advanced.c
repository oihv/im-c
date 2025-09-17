#include "unity.h"
#include "mock_websocket_server.h"
#include "websocket_service.h"
#include "message_types.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static MockWebSocketServer *mock_server = NULL;

void setUp(void) {
    /* Create mock server for each test */
    mock_server = mock_server_create(MOCK_SERVER_PORT);
    TEST_ASSERT_NOT_NULL(mock_server);
    
    /* Initialize websocket service */
    TEST_ASSERT_TRUE(websocket_service_init());
    
    /* Start mock server */
    TEST_ASSERT_TRUE(mock_server_start(mock_server));
}

void tearDown(void) {
    /* Clean up websocket service */
    websocket_service_cleanup();
    
    /* Stop and destroy mock server */
    if (mock_server) {
        mock_server_stop(mock_server);
        mock_server_destroy(mock_server);
        mock_server = NULL;
    }
}

void test_mock_server_basic_functionality(void) {
    int client_id;
    size_t message_count;
    MockMessage *messages;
    
    /* Test basic server operations */
    TEST_ASSERT_TRUE(mock_server->running);
    TEST_ASSERT_EQUAL_INT(MOCK_SERVER_PORT, mock_server->port);
    
    /* Test client connection */
    client_id = mock_server_simulate_client_connect(mock_server);
    TEST_ASSERT_TRUE(client_id >= 0);
    TEST_ASSERT_TRUE(mock_server->clients[client_id].connected);
    TEST_ASSERT_EQUAL_INT(1, mock_server->client_count);
    
    /* Test message sending */
    TEST_ASSERT_TRUE(mock_server_simulate_client_send_message(mock_server, client_id, "Hello World"));
    
    /* Check message was stored */
    messages = mock_server_get_client_messages(mock_server, client_id, &message_count);
    TEST_ASSERT_NOT_NULL(messages);
    TEST_ASSERT_EQUAL_INT(1, message_count);
    TEST_ASSERT_EQUAL_STRING("Hello World", messages[0].message);
    
    /* Test client disconnection */
    mock_server_simulate_client_disconnect(mock_server, client_id);
    TEST_ASSERT_FALSE(mock_server->clients[client_id].connected);
    TEST_ASSERT_EQUAL_INT(0, mock_server->client_count);
}

void test_mock_server_multiple_clients(void) {
    int client1, client2;
    size_t count1, count2;
    MockMessage *messages1, *messages2;
    
    /* Connect two clients */
    client1 = mock_server_simulate_client_connect(mock_server);
    client2 = mock_server_simulate_client_connect(mock_server);
    TEST_ASSERT_TRUE(client1 >= 0);
    TEST_ASSERT_TRUE(client2 >= 0);
    TEST_ASSERT_NOT_EQUAL(client1, client2);
    TEST_ASSERT_EQUAL_INT(2, mock_server->client_count);
    
    /* Client 1 sends message */
    TEST_ASSERT_TRUE(mock_server_simulate_client_send_message(mock_server, client1, "Message from client 1"));
    
    /* Both clients should have the message (echo enabled by default) */
    messages1 = mock_server_get_client_messages(mock_server, client1, &count1);
    messages2 = mock_server_get_client_messages(mock_server, client2, &count2);
    
    TEST_ASSERT_EQUAL_INT(1, count1); /* Original sender receives their own message */
    TEST_ASSERT_EQUAL_INT(1, count2); /* Other client receives echoed message */
    TEST_ASSERT_EQUAL_STRING("Message from client 1", messages1[0].message);
    TEST_ASSERT_EQUAL_STRING("Message from client 1", messages2[0].message);
}

void test_mock_server_connection_rejection(void) {
    int client_id;
    
    /* Disable connection acceptance */
    mock_server_set_accept_connections(mock_server, false);
    
    /* Try to connect - should fail */
    client_id = mock_server_simulate_client_connect(mock_server);
    TEST_ASSERT_EQUAL_INT(-1, client_id);
    TEST_ASSERT_EQUAL_INT(0, mock_server->client_count);
    TEST_ASSERT_EQUAL_INT(1, mock_server->failed_connections);
}

void test_mock_server_connection_error_simulation(void) {
    int client_id;
    
    /* Enable connection error simulation */
    mock_server_set_simulate_connection_error(mock_server, true);
    
    /* Try to connect - should fail */
    client_id = mock_server_simulate_client_connect(mock_server);
    TEST_ASSERT_EQUAL_INT(-1, client_id);
    TEST_ASSERT_EQUAL_INT(0, mock_server->client_count);
    TEST_ASSERT_EQUAL_INT(1, mock_server->failed_connections);
}

void test_mock_server_message_loss_simulation(void) {
    int client_id;
    int successful_messages = 0;
    int i;
    
    /* Connect client */
    client_id = mock_server_simulate_client_connect(mock_server);
    TEST_ASSERT_TRUE(client_id >= 0);
    
    /* Enable message loss simulation */
    mock_server_set_simulate_message_loss(mock_server, true);
    
    /* Send multiple messages - some should be lost */
    for (i = 0; i < 20; i++) {
        char message[64];
        sprintf(message, "Message %d", i);
        if (mock_server_simulate_client_send_message(mock_server, client_id, message)) {
            successful_messages++;
        }
    }
    
    /* We should have lost some messages (not all 20 should succeed) */
    TEST_ASSERT_TRUE(successful_messages < 20);
    TEST_ASSERT_TRUE(successful_messages > 0); /* But not all should be lost */
}

void test_mock_server_broadcast_functionality(void) {
    int client1, client2;
    size_t count1, count2, broadcast_count;
    MockMessage *messages1, *messages2, *broadcast_history;
    
    /* Connect two clients */
    client1 = mock_server_simulate_client_connect(mock_server);
    client2 = mock_server_simulate_client_connect(mock_server);
    
    /* Broadcast a message to all clients */
    mock_server_broadcast_message(mock_server, "Broadcast message");
    
    /* Both clients should receive the message */
    messages1 = mock_server_get_client_messages(mock_server, client1, &count1);
    messages2 = mock_server_get_client_messages(mock_server, client2, &count2);
    broadcast_history = mock_server_get_broadcast_history(mock_server, &broadcast_count);
    
    TEST_ASSERT_EQUAL_INT(1, count1);
    TEST_ASSERT_EQUAL_INT(1, count2);
    TEST_ASSERT_EQUAL_INT(1, broadcast_count);
    TEST_ASSERT_EQUAL_STRING("Broadcast message", messages1[0].message);
    TEST_ASSERT_EQUAL_STRING("Broadcast message", messages2[0].message);
    TEST_ASSERT_EQUAL_STRING("Broadcast message", broadcast_history[0].message);
}

void test_mock_server_statistics(void) {
    int client_id;
    int connections, messages_sent, messages_received;
    
    /* Reset stats */
    mock_server_reset_stats(mock_server);
    mock_server_get_stats(mock_server, &connections, &messages_sent, &messages_received);
    TEST_ASSERT_EQUAL_INT(0, connections);
    TEST_ASSERT_EQUAL_INT(0, messages_sent);
    TEST_ASSERT_EQUAL_INT(0, messages_received);
    
    /* Connect client and send message */
    client_id = mock_server_simulate_client_connect(mock_server);
    TEST_ASSERT_TRUE(mock_server_simulate_client_send_message(mock_server, client_id, "Test message"));
    
    /* Check updated stats */
    mock_server_get_stats(mock_server, &connections, &messages_sent, &messages_received);
    TEST_ASSERT_EQUAL_INT(1, connections);
    TEST_ASSERT_EQUAL_INT(0, messages_sent); /* No other clients to echo to */
    TEST_ASSERT_EQUAL_INT(1, messages_received);
}

void test_mock_server_echo_disable(void) {
    int client1, client2;
    size_t count1, count2;
    MockMessage *messages1, *messages2;
    
    /* Connect two clients */
    client1 = mock_server_simulate_client_connect(mock_server);
    client2 = mock_server_simulate_client_connect(mock_server);
    
    /* Disable echo */
    mock_server_set_echo_messages(mock_server, false);
    
    /* Client 1 sends message */
    TEST_ASSERT_TRUE(mock_server_simulate_client_send_message(mock_server, client1, "No echo message"));
    
    /* Only sender should have the message */
    messages1 = mock_server_get_client_messages(mock_server, client1, &count1);
    messages2 = mock_server_get_client_messages(mock_server, client2, &count2);
    
    TEST_ASSERT_EQUAL_INT(1, count1); /* Original sender has message */
    TEST_ASSERT_EQUAL_INT(0, count2); /* Other client doesn't receive echo */
}

void test_websocket_integration_with_mock_server(void) {
    /* This test would require modifying websocket_service to use our mock server
     * For now, we test the interface compatibility */
    WebSocketData *data;
    Message test_message = {0};
    
    /* Test service initialization */
    TEST_ASSERT_TRUE(websocket_service_init());
    
    /* Test service update */
    data = websocket_service_update();
    TEST_ASSERT_NOT_NULL(data);
    
    /* Test message creation and sending */
    test_message.type = MSG_TYPE_CHAT;
    strcpy(test_message.username, "TestUser");
    strcpy(test_message.content, "Integration test message");
    test_message.timestamp = 1234567890;
    
    /* This would normally send to real server, but in testing we verify the interface */
    websocket_service_send_message(&test_message);
    websocket_service_send_text("TestUser", "Simple text message");
    
    /* Cleanup */
    websocket_service_cleanup();
}

int main(void) {
    UNITY_BEGIN();
    
    /* Mock server functionality tests */
    RUN_TEST(test_mock_server_basic_functionality);
    RUN_TEST(test_mock_server_multiple_clients);
    RUN_TEST(test_mock_server_connection_rejection);
    RUN_TEST(test_mock_server_connection_error_simulation);
    RUN_TEST(test_mock_server_message_loss_simulation);
    RUN_TEST(test_mock_server_broadcast_functionality);
    RUN_TEST(test_mock_server_statistics);
    RUN_TEST(test_mock_server_echo_disable);
    
    /* Integration tests */
    RUN_TEST(test_websocket_integration_with_mock_server);
    
    return UNITY_END();
}