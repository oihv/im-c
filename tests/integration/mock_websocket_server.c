#include "mock_websocket_server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

MockWebSocketServer* mock_server_create(int port) {
    MockWebSocketServer *server = malloc(sizeof(MockWebSocketServer));
    if (!server) return NULL;
    
    memset(server, 0, sizeof(MockWebSocketServer));
    server->port = port;
    server->running = false;
    server->should_accept_connections = true;
    server->should_echo_messages = true;
    server->should_simulate_connection_error = false;
    server->should_simulate_message_loss = false;
    
    // Initialize clients
    for (int i = 0; i < MOCK_MAX_CLIENTS; i++) {
        server->clients[i].client_id = i;
        server->clients[i].connected = false;
        server->clients[i].max_messages = 100;
        server->clients[i].received_messages = malloc(sizeof(MockMessage) * server->clients[i].max_messages);
        server->clients[i].message_count = 0;
    }
    
    // Initialize broadcast history
    server->max_broadcast_history = 200;
    server->broadcast_history = malloc(sizeof(MockMessage) * server->max_broadcast_history);
    server->broadcast_count = 0;
    
    return server;
}

void mock_server_destroy(MockWebSocketServer *server) {
    if (!server) return;
    
    // Clean up client message buffers
    for (int i = 0; i < MOCK_MAX_CLIENTS; i++) {
        if (server->clients[i].received_messages) {
            free(server->clients[i].received_messages);
        }
    }
    
    // Clean up broadcast history
    if (server->broadcast_history) {
        free(server->broadcast_history);
    }
    
    free(server);
}

bool mock_server_start(MockWebSocketServer *server) {
    if (!server || server->running) return false;
    
    server->running = true;
    server->connection_attempts = 0;
    server->successful_connections = 0;
    server->failed_connections = 0;
    server->messages_sent = 0;
    server->messages_received = 0;
    
    return true;
}

void mock_server_stop(MockWebSocketServer *server) {
    if (!server) return;
    
    server->running = false;
    
    // Disconnect all clients
    for (int i = 0; i < MOCK_MAX_CLIENTS; i++) {
        if (server->clients[i].connected) {
            server->clients[i].connected = false;
        }
    }
    server->client_count = 0;
}

void mock_server_update(MockWebSocketServer *server) {
    if (!server || !server->running) return;
    
    // Simulate server processing - in a real implementation this would
    // handle network events, but for mocking we just update timestamps
    // and process any queued operations
    usleep(1000); // 1ms delay to simulate processing time
}

int mock_server_simulate_client_connect(MockWebSocketServer *server) {
    int i;
    if (!server || !server->running) return -1;
    
    server->connection_attempts++;
    
    if (!server->should_accept_connections) {
        server->failed_connections++;
        return -1;
    }
    
    if (server->should_simulate_connection_error) {
        server->failed_connections++;
        return -1;
    }
    
    /* Find available client slot */
    for (i = 0; i < MOCK_MAX_CLIENTS; i++) {
        if (!server->clients[i].connected) {
            server->clients[i].connected = true;
            server->clients[i].message_count = 0;
            server->client_count++;
            server->successful_connections++;
            return i;
        }
    }
    
    server->failed_connections++;
    return -1; /* No available slots */
}

void mock_server_simulate_client_disconnect(MockWebSocketServer *server, int client_id) {
    if (!server || client_id < 0 || client_id >= MOCK_MAX_CLIENTS) return;
    
    if (server->clients[client_id].connected) {
        server->clients[client_id].connected = false;
        server->client_count--;
    }
}

bool mock_server_simulate_client_send_message(MockWebSocketServer *server, int client_id, const char *message) {
    MockClient *client;
    MockMessage *msg;
    struct timeval tv;
    int i;
    
    if (!server || !message || client_id < 0 || client_id >= MOCK_MAX_CLIENTS) return false;
    if (!server->clients[client_id].connected) return false;
    
    if (server->should_simulate_message_loss) {
        /* Randomly drop 20% of messages */
        if (rand() % 5 == 0) return false;
    }
    
    client = &server->clients[client_id];
    
    /* Store received message */
    if (client->message_count < client->max_messages) {
        msg = &client->received_messages[client->message_count];
        strncpy(msg->message, message, MOCK_MAX_MESSAGE_SIZE - 1);
        msg->message[MOCK_MAX_MESSAGE_SIZE - 1] = '\0';
        msg->length = strlen(msg->message);
        
        gettimeofday(&tv, NULL);
        msg->timestamp = tv.tv_sec * 1000000ULL + tv.tv_usec;
        
        client->message_count++;
        server->messages_received++;
    }
    
    /* Echo message to all other clients if echo is enabled */
    if (server->should_echo_messages) {
        for (i = 0; i < MOCK_MAX_CLIENTS; i++) {
            if (i != client_id && server->clients[i].connected) {
                MockClient *other_client = &server->clients[i];
                if (other_client->message_count < other_client->max_messages) {
                    MockMessage *echo_msg = &other_client->received_messages[other_client->message_count];
                    strncpy(echo_msg->message, message, MOCK_MAX_MESSAGE_SIZE - 1);
                    echo_msg->message[MOCK_MAX_MESSAGE_SIZE - 1] = '\0';
                    echo_msg->length = strlen(echo_msg->message);
                    
                    gettimeofday(&tv, NULL);
                    echo_msg->timestamp = tv.tv_sec * 1000000ULL + tv.tv_usec;
                    
                    other_client->message_count++;
                    server->messages_sent++;
                }
            }
        }
    }
    
    /* Add to broadcast history */
    if (server->broadcast_count < server->max_broadcast_history) {
        MockMessage *broadcast_msg = &server->broadcast_history[server->broadcast_count];
        strncpy(broadcast_msg->message, message, MOCK_MAX_MESSAGE_SIZE - 1);
        broadcast_msg->message[MOCK_MAX_MESSAGE_SIZE - 1] = '\0';
        broadcast_msg->length = strlen(broadcast_msg->message);
        
        gettimeofday(&tv, NULL);
        broadcast_msg->timestamp = tv.tv_sec * 1000000ULL + tv.tv_usec;
        
        server->broadcast_count++;
    }
    
    return true;
}

void mock_server_set_accept_connections(MockWebSocketServer *server, bool accept) {
    if (server) {
        server->should_accept_connections = accept;
    }
}

void mock_server_set_echo_messages(MockWebSocketServer *server, bool echo) {
    if (server) {
        server->should_echo_messages = echo;
    }
}

void mock_server_set_simulate_connection_error(MockWebSocketServer *server, bool simulate) {
    if (server) {
        server->should_simulate_connection_error = simulate;
    }
}

void mock_server_set_simulate_message_loss(MockWebSocketServer *server, bool simulate) {
    if (server) {
        server->should_simulate_message_loss = simulate;
    }
}

void mock_server_inject_message(MockWebSocketServer *server, const char *message) {
    int i;
    MockClient *client;
    MockMessage *msg;
    struct timeval tv;
    
    if (!server || !message) return;
    
    /* Send message to all connected clients */
    for (i = 0; i < MOCK_MAX_CLIENTS; i++) {
        if (server->clients[i].connected) {
            client = &server->clients[i];
            if (client->message_count < client->max_messages) {
                msg = &client->received_messages[client->message_count];
                strncpy(msg->message, message, MOCK_MAX_MESSAGE_SIZE - 1);
                msg->message[MOCK_MAX_MESSAGE_SIZE - 1] = '\0';
                msg->length = strlen(msg->message);
                
                gettimeofday(&tv, NULL);
                msg->timestamp = tv.tv_sec * 1000000ULL + tv.tv_usec;
                
                client->message_count++;
                server->messages_sent++;
            }
        }
    }
}

void mock_server_broadcast_message(MockWebSocketServer *server, const char *message) {
    struct timeval tv;
    MockMessage *broadcast_msg;
    
    if (!server || !message) return;
    
    /* Send to all clients via inject_message */
    mock_server_inject_message(server, message);
    
    /* Record in broadcast history */
    if (server->broadcast_count < server->max_broadcast_history) {
        broadcast_msg = &server->broadcast_history[server->broadcast_count];
        strncpy(broadcast_msg->message, message, MOCK_MAX_MESSAGE_SIZE - 1);
        broadcast_msg->message[MOCK_MAX_MESSAGE_SIZE - 1] = '\0';
        broadcast_msg->length = strlen(broadcast_msg->message);
        
        gettimeofday(&tv, NULL);
        broadcast_msg->timestamp = tv.tv_sec * 1000000ULL + tv.tv_usec;
        
        server->broadcast_count++;
    }
}

bool mock_server_wait_for_client_connection(MockWebSocketServer *server, int timeout_ms) {
    int initial_connections;
    int elapsed;
    
    if (!server) return false;
    
    initial_connections = server->successful_connections;
    elapsed = 0;
    
    while (elapsed < timeout_ms) {
        mock_server_update(server);
        if (server->successful_connections > initial_connections) {
            return true;
        }
        usleep(1000); /* 1ms */
        elapsed++;
    }
    
    return false;
}

bool mock_server_wait_for_message(MockWebSocketServer *server, int client_id, const char *expected_message, int timeout_ms) {
    int elapsed;
    MockClient *client;
    size_t i;
    
    if (!server || client_id < 0 || client_id >= MOCK_MAX_CLIENTS || !expected_message) return false;
    if (!server->clients[client_id].connected) return false;
    
    elapsed = 0;
    client = &server->clients[client_id];
    
    while (elapsed < timeout_ms) {
        mock_server_update(server);
        
        /* Check if we have new messages */
        for (i = 0; i < client->message_count; i++) {
            if (strstr(client->received_messages[i].message, expected_message) != NULL) {
                return true;
            }
        }
        
        usleep(1000); /* 1ms */
        elapsed++;
    }
    
    return false;
}

MockMessage* mock_server_get_client_messages(MockWebSocketServer *server, int client_id, size_t *count) {
    if (!server || client_id < 0 || client_id >= MOCK_MAX_CLIENTS || !count) {
        if (count) *count = 0;
        return NULL;
    }
    
    *count = server->clients[client_id].message_count;
    return server->clients[client_id].received_messages;
}

MockMessage* mock_server_get_broadcast_history(MockWebSocketServer *server, size_t *count) {
    if (!server || !count) {
        if (count) *count = 0;
        return NULL;
    }
    
    *count = server->broadcast_count;
    return server->broadcast_history;
}

void mock_server_reset_stats(MockWebSocketServer *server) {
    if (!server) return;
    
    server->connection_attempts = 0;
    server->successful_connections = 0;
    server->failed_connections = 0;
    server->messages_sent = 0;
    server->messages_received = 0;
    
    // Reset client message counts
    for (int i = 0; i < MOCK_MAX_CLIENTS; i++) {
        server->clients[i].message_count = 0;
    }
    
    server->broadcast_count = 0;
}

void mock_server_get_stats(MockWebSocketServer *server, int *connections, int *messages_sent, int *messages_received) {
    if (!server) return;
    
    if (connections) *connections = server->successful_connections;
    if (messages_sent) *messages_sent = server->messages_sent;
    if (messages_received) *messages_received = server->messages_received;
}
