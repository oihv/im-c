#include "websocket_service.h"
#include <libwebsockets.h>
#include <stdint.h>
#include <string.h>
#include "../clay.h"

static struct lws_context *ws_context = NULL;
static struct my_conn {
  lws_sorted_usec_list_t sul;
  struct lws *wsi;
  uint16_t retry_count;
  char send_buffer[256];
  bool has_data_to_send;
} ws_connection;

static WebSocketData ws_data = {0};

// Retry policy
static const uint32_t backoff_ms[] = {1000, 2000, 3000, 4000, 5000};
static const lws_retry_bo_t retry = {
    .retry_ms_table = backoff_ms,
    .retry_ms_table_count = LWS_ARRAY_SIZE(backoff_ms),
    .conceal_count = LWS_ARRAY_SIZE(backoff_ms),
    .secs_since_valid_ping = 3,
    .secs_since_valid_hangup = 10,
    .jitter_percent = 20,
};

static void connect_client(lws_sorted_usec_list_t *sul) {
  // What does container_of macro do?
  struct my_conn *m = lws_container_of(sul, struct my_conn, sul);
  struct lws_client_connect_info i = {0};

  i.context = ws_context;
  i.port = 7681;
  i.address = "localhost";
  i.path = "/";
  i.host = i.address;
  i.origin = i.address;
  i.ssl_connection = 0; // No SSL
  i.protocol = "lws-minimal";
  i.local_protocol_name = "lws-minimal-client";
  i.pwsi = &m->wsi;
  i.retry_and_idle_policy = &retry;
  i.userdata = m;

  if (!lws_client_connect_via_info(&i)) {
    if (lws_retry_sul_schedule(ws_context, 0, sul, &retry, connect_client,
                               &m->retry_count)) {
      strcpy(ws_data.connection_status, "Connection failed");
      ws_data.connected = false;
    }
  }
}

static int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
                            void *user, void *in, size_t len) {
  switch (reason) {
  case LWS_CALLBACK_CLIENT_ESTABLISHED:
    ws_data.connected = true;
    strcpy(ws_data.connection_status, "Connected");
    break;

  case LWS_CALLBACK_CLIENT_RECEIVE:
    // Copy received message to our data structure
    if (len < sizeof(ws_data.message) - 1) {
      memcpy(ws_data.message, in, len);
      ws_data.message[len] = '\0';
      ws_data.has_new_message = true;
    }
    break;

  case LWS_CALLBACK_CLIENT_WRITEABLE:
    if (ws_connection.has_data_to_send) {
        unsigned char buf[LWS_PRE + 512];
        unsigned char *p = &buf[LWS_PRE];

        int len = sprintf((char *)p, "%s", ws_connection.send_buffer);

        if (lws_write(wsi, p, len, LWS_WRITE_TEXT) < len) {
            return -1;
        }

        ws_connection.has_data_to_send = false;  // Clear flag
    }
    break;

  case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    ws_data.connected = false;
    strcpy(ws_data.connection_status, "Connection error");
    goto do_retry;

  case LWS_CALLBACK_CLIENT_CLOSED:
    ws_data.connected = false;
    strcpy(ws_data.connection_status, "Disconnected");
    goto do_retry;

  default:
    break;
  }
  return 0;

do_retry:
  if (lws_retry_sul_schedule_retry_wsi(wsi, &ws_connection.sul, connect_client,
                                       &ws_connection.retry_count)) {
    strcpy(ws_data.connection_status, "Retry failed");
  }
  return 0;
}

static const struct lws_protocols protocols[] = {
    {"lws-minimal-client", callback_minimal, 0, 0, 0, NULL, 0},
    LWS_PROTOCOL_LIST_TERM
};

bool websocket_service_init(void) {
  struct lws_context_creation_info info = {0};

  info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
  info.port = CONTEXT_PORT_NO_LISTEN;
  info.protocols = protocols;

  ws_context = lws_create_context(&info);
  if (!ws_context)
    return false;

  // Start connection attempt
  lws_sul_schedule(ws_context, 0, &ws_connection.sul, connect_client, 1);

  strcpy(ws_data.connection_status, "Connecting...");
  return true;
}

WebSocketData *websocket_service_update(void) {
  if (ws_context) {
    // Non-blocking service call
    lws_service(ws_context, 0);
  }
  return &ws_data;
}

void websocket_service_send(const char* message) {
    if (ws_connection.wsi && strlen(message) < sizeof(ws_connection.send_buffer)) {
        strcpy(ws_connection.send_buffer, message);
        ws_connection.has_data_to_send = true;
        lws_callback_on_writable(ws_connection.wsi);  // Request write permission
    }
}

void websocket_service_cleanup(void) {
  if (ws_context) {
    lws_context_destroy(ws_context);
    ws_context = NULL;
  }
}
