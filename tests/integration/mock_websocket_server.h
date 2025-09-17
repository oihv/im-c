#ifndef MOCK_WEBSOCKET_SERVER_H
#define MOCK_WEBSOCKET_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MOCK_MAX_CLIENTS 10
#define MOCK_MAX_MESSAGE_SIZE 2048
#define MOCK_SERVER_PORT 7682

typedef struct {
    char message[MOCK_MAX_MESSAGE_SIZE];
    size_t length;
    uint64_t timestamp;
} MockMessage;

typedef struct {
    int client_id;
    bool connected;
    MockMessage *received_messages;
    size_t message_count;
    size_t max_messages;
} MockClient;

typedef struct {
    int port;
    bool running;
    bool should_accept_connections;
    bool should_echo_messages;
    bool should_simulate_connection_error;
    bool should_simulate_message_loss;
    
    MockClient clients[MOCK_MAX_CLIENTS];
    int client_count;
    
    MockMessage *broadcast_history;
    size_t broadcast_count;
    size_t max_broadcast_history;
    
    // Statistics for testing
    int connection_attempts;
    int successful_connections;
    int failed_connections;
    int messages_sent;
    int messages_received;
} MockWebSocketServer;

// Server lifecycle
MockWebSocketServer* mock_server_create(int port);
void mock_server_destroy(MockWebSocketServer *server);
bool mock_server_start(MockWebSocketServer *server);
void mock_server_stop(MockWebSocketServer *server);
void mock_server_update(MockWebSocketServer *server);

// Client simulation
int mock_server_simulate_client_connect(MockWebSocketServer *server);
void mock_server_simulate_client_disconnect(MockWebSocketServer *server, int client_id);
bool mock_server_simulate_client_send_message(MockWebSocketServer *server, int client_id, const char *message);

// Server behavior control
void mock_server_set_accept_connections(MockWebSocketServer *server, bool accept);
void mock_server_set_echo_messages(MockWebSocketServer *server, bool echo);
void mock_server_set_simulate_connection_error(MockWebSocketServer *server, bool simulate);
void mock_server_set_simulate_message_loss(MockWebSocketServer *server, bool simulate);

// Message injection for testing
void mock_server_inject_message(MockWebSocketServer *server, const char *message);
void mock_server_broadcast_message(MockWebSocketServer *server, const char *message);

// Test utilities
bool mock_server_wait_for_client_connection(MockWebSocketServer *server, int timeout_ms);
bool mock_server_wait_for_message(MockWebSocketServer *server, int client_id, const char *expected_message, int timeout_ms);
MockMessage* mock_server_get_client_messages(MockWebSocketServer *server, int client_id, size_t *count);
MockMessage* mock_server_get_broadcast_history(MockWebSocketServer *server, size_t *count);

// Statistics
void mock_server_reset_stats(MockWebSocketServer *server);
void mock_server_get_stats(MockWebSocketServer *server, int *connections, int *messages_sent, int *messages_received);

#endif // MOCK_WEBSOCKET_SERVER_H