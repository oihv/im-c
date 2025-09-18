#include "websocket_service.h"
#include <stdio.h>
#include <string.h>

// Stub implementations for when networking is disabled (e.g., Windows builds without libwebsockets)

static WebSocketData stub_ws_data = {0};

bool websocket_service_init(void) {
    printf("Warning: Networking disabled - websocket_service_init called\n");
    stub_ws_data.connected = false;
    stub_ws_data.error = false;
    stub_ws_data.has_new_message = false;
    strcpy(stub_ws_data.connection_status, "Networking disabled");
    return true;
}

bool websocket_service_connect() {
    printf("Warning: Networking disabled - websocket_service_connect called\n");
    return false;
}

bool websocket_should_close() {
    return false;
}

WebSocketData* websocket_service_update(void) {
    return &stub_ws_data;
}

void websocket_service_send_message(const Message* message) {
    printf("Warning: Networking disabled - cannot send message\n");
}

void websocket_service_send_text(const char* username, const char* text) {
    printf("Warning: Networking disabled - cannot send text: %s\n", text);
}

void websocket_service_cleanup(void) {
    printf("Warning: Networking disabled - websocket_service_cleanup called\n");
}