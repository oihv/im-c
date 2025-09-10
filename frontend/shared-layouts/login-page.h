#ifndef LOGIN_PAGE
#define LOGIN_PAGE
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../network/websocket_service.h"

typedef enum {
  Disconnected,
  InitiateConnect,
  Connecting,
  Connected,
  ConnectionError
} ConnectStatus;

typedef struct
{
    bool loggedIn;
    ConnectStatus status;
    char username[32];
    char ipaddr[32];
    char port[32];

    uint16_t frameCount;

    char username_buf[32];
    size_t username_len;
    char ipaddr_buf[32];
    size_t ipaddr_len;

    bool focusList[2];
    size_t focus_len;

    my_conn* ws_conn;
    // LoginPage_Arena arena;
} LoginPage_Data;
#endif
