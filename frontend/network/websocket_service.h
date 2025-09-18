#ifndef WEBSOCKET_SERVICE_H
#define WEBSOCKET_SERVICE_H

#ifndef DISABLE_NETWORKING
#if defined(_WIN32)           
	#define NOGDI             // All GDI defines and routines
	#define NOUSER            // All USER defines and routines
#endif

// #include <windows.h> // or any library that uses Windows.h
#include <libwebsockets.h>

#if defined(_WIN32)           // raylib uses these names as function parameters
	#undef near
	#undef far
#endif
#endif // DISABLE_NETWORKING

#include <stdbool.h>
#include "../clay.h"
#include "message_types.h"

typedef struct {
  MessageList* messages;
  bool has_new_message;
  bool connected;
  bool error;
  char connection_status[100];
} WebSocketData;

#ifndef DISABLE_NETWORKING
typedef struct {
  lws_sorted_usec_list_t sul;
  struct lws *wsi;
  uint16_t retry_count;
  char send_buffer[256];
  bool has_data_to_send;
  char* ipaddr;
  int port;
  bool error;
} my_conn;
#else
// Stub definition when networking is disabled
typedef struct {
  char* ipaddr;
  int port;
  bool error;
} my_conn;
#endif // DISABLE_NETWORKING

// Initialize the websocket service
bool websocket_service_init(void);

// initiate service connection
bool websocket_service_connect();

// Libuv signal
bool websocket_should_close();

// Call this every frame - returns updated data
WebSocketData* websocket_service_update(void);

// Send a message to the server
void websocket_service_send_message(const Message* message);

// Send a simple text message
void websocket_service_send_text(const char* username, const char* text);

// Cleanup
void websocket_service_cleanup(void);

#endif
