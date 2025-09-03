#ifndef WEBSOCKET_SERVICE_H
#define WEBSOCKET_SERVICE_H

#include <libwebsockets.h>
#include <stdbool.h>

typedef struct {
  char message[512];
  bool has_new_message;
  bool connected;
  char connection_status[100];
} WebSocketData;

// Initialize the websocket service
bool websocket_service_init(void);

// Libuv signal
bool websocket_should_close();

// Call this every frame - returns updated data
WebSocketData* websocket_service_update(void);

// Send a message to the server
void websocket_service_send(const char* message);

// Cleanup
void websocket_service_cleanup(void);

#endif
