#ifndef WEBSOCKET_SERVICE_H
#define WEBSOCKET_SERVICE_H
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
#include <stdbool.h>
#include "../clay.h"

typedef struct {
  char message[512];
  bool has_new_message;
  bool connected;
  char connection_status[100];
} WebSocketData;

static struct my_conn {
  lws_sorted_usec_list_t sul;
  struct lws *wsi;
  uint16_t retry_count;
  char send_buffer[256];
  bool has_data_to_send;
  Clay_String ipaddr;
  Clay_String port;
} ws_connection;

typedef struct my_conn my_conn;

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
