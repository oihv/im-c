#ifndef LOGIN_PAGE
#define LOGIN_PAGE
#include <stddef.h>
#include <stdint.h>
typedef struct
{
    bool loggedIn;
    char username[32];
    char password[32];

    uint16_t frameCount;

    char username_buf[32];
    size_t username_len;
    char pass_buf[32];
    size_t pass_len;

    bool focusList[2];
    size_t focus_len;

    // LoginPage_Arena arena;
} LoginPage_Data;
#endif
