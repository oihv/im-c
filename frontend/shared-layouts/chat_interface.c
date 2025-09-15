#include "../clay.h"
#include "../components/textbox.h"
#include "../network/websocket_service.h"
#include "../network/message_types.h"
#include "../renderers/raylib/raylib.h"
#include "login_page.h"
#include "mainpage.h"
#include <stdio.h>
#include <stdlib.h>
#include "mainpage.h"
#include <string.h>
#include "../renderers/raylib/raylib.h"
#include "../components/textbox.h"

void RenderHeaderButton(Clay_String text)
{
    CLAY({.layout = {.padding = {16, 16, 8, 8}},
          .backgroundColor = {140, 140, 140, 255},
          .cornerRadius = CLAY_CORNER_RADIUS(5)})
    {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                          .fontSize = 16,
                                          .textColor = {255, 255, 255, 255}}));
    }
}

void RenderDropdownMenuItem(Clay_String text)
{
    CLAY({.layout = {.padding = CLAY_PADDING_ALL(16)}})
    {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                          .fontSize = 16,
                                          .textColor = {255, 255, 255, 255}}));
    }
}

// Add this function to calculate responsive font size
int GetResponsiveFontSize(int baseFontSize, int windowWidth, int windowHeight) {
    // Base calculation on window width (you can adjust the formula)
    float scaleFactor = (float)windowWidth / 1200.0f; // 1200px as reference width
    
    // Clamp the scale factor between 0.8 and 1.5 for reasonable limits
    if (scaleFactor < 0.8f) scaleFactor = 0.8f;
    if (scaleFactor > 1.5f) scaleFactor = 1.5f;
    
    return (int)(baseFontSize * scaleFactor);
}

typedef struct
{
    Clay_String title;
    Clay_String contents;
} Document;

typedef struct
{
    Document *documents;
    uint32_t length;
} DocumentArray;

Document documentsRaw[5];

DocumentArray documents = {
    .length = 5,
    .documents = documentsRaw};

typedef struct
{
    Clay_String sender;
    Clay_String text;
    bool isSender; // true = user, false = bot/system
} ChatMessage;

#define MAX_MESSAGES 100
ChatMessage chatMessages[MAX_MESSAGES];
int chatMessageCount = 0;

static inline Clay_String ClayStr(const char *s)
{
    return (Clay_String){s, (int)strlen(s)}; // positional init: {ptr, length}
}

void AddBotReply(const char *replyText)
{
    if (chatMessageCount < MAX_MESSAGES)
    {
        chatMessages[chatMessageCount].sender = CLAY_STRING("Bot");
        chatMessages[chatMessageCount].text = (Clay_String){
            .chars = replyText,
            .length = (int)strlen(replyText)};
        chatMessages[chatMessageCount].isSender = false; // bot/system
        chatMessageCount++;
    }
}

void AddUserMessage(const char *userText)
{
    if (chatMessageCount < MAX_MESSAGES)
    {
        chatMessages[chatMessageCount].sender = CLAY_STRING("Ben"); // or use ClayStr("Ben") if dynamic
        chatMessages[chatMessageCount].text = (Clay_String){
            .chars = userText,
            .length = (int)strlen(userText)};
        chatMessages[chatMessageCount].isSender = true; // user
        chatMessageCount++;
    }
}

typedef struct {
  intptr_t offset;
  intptr_t memory;
} ChatApp_Arena;

typedef struct {
  int32_t selectedDocumentIndex;
  float yOffset;
  LoginPage_Data *login_credentials;

  char message_buffer[256];
  size_t message_len;
  uint16_t frameCount;
  bool focusList;

  // WebSocket data
  WebSocketData *ws_data;
  int last_processed_message_count;
  
  // Auto-scroll animation
  bool auto_scrolling;
  float scroll_target;
  float scroll_velocity;
  int scroll_delay_frames; // Add delay before starting scroll
  
  // Manual scroll state
  ScrollState manual_scroll_state;

  ChatApp_Arena frameArena;
} ChatApp_Data;

typedef struct {
  ChatApp_Data *app_data;
  bool is_dragging;
  float scroll_bar_height;
  float scroll_bar_y;
} ScrollBarData;

typedef struct {
  int32_t requestedDocumentIndex;
  int32_t *selectedDocumentIndex;
} SidebarClickData;

typedef struct {
  int dummy; // placeholder for future use
  ChatApp_Data *app_data; // Add reference to chat app data
} SendClickData;

void SendMessage(ChatApp_Data* data) {
  // Null-terminate the message buffer
  data->message_buffer[data->message_len] = '\0';
  
  // Send via WebSocket using the username from login credentials
  websocket_service_send_text(data->login_credentials->username_buf, 
                              data->message_buffer);
  
  // Clear the message buffer
  memset(data->message_buffer, 0, sizeof(data->message_buffer));
  data->message_len = 0;
}

// Handle manual scroll velocity
void UpdateManualScrollVelocity(ChatApp_Data *data) {
  // Get the current time (using frame count as proxy)
  float current_time = (float)data->frameCount * 0.016f; // Approximate 60fps
  
  // Check if we're manually scrolling
  Clay_ElementId mainContentId = CLAY_ID("MainContent");
  Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(mainContentId);
  
  if (scrollData.found && scrollData.scrollPosition) {
    // Detect manual scroll input (mouse wheel or dragging)
    Vector2 scrollDelta = GetMouseWheelMoveV();
    bool hasScrollInput = (fabs(scrollDelta.y) > 0.01f);
    
    if (hasScrollInput) {
      data->manual_scroll_state.is_manual_scrolling = true;
      data->manual_scroll_state.last_manual_scroll_time = current_time;
      data->auto_scrolling = false; // Stop auto-scroll when manually scrolling
    }
}

// Convert WebSocket messages to ChatMessage format for rendering
void UpdateAutoScroll(ChatApp_Data *data) {
  // Handle scroll delay
  if (data->scroll_delay_frames > 0) {
    data->scroll_delay_frames--;
    return;
  }
  
  if (!data->auto_scrolling) return;
  
  Clay_ElementId mainContentId = CLAY_ID("MainContent");
  Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(mainContentId);
  
  if (scrollData.found && scrollData.scrollPosition) {
    // If scroll_target is still the placeholder, calculate the real target
    if (data->scroll_target <= -999000.0f) {
      float maxScrollY = scrollData.contentDimensions.height - scrollData.scrollContainerDimensions.height;
      if (maxScrollY > 0) {
        data->scroll_target = -maxScrollY;
      } else {
        data->scroll_target = 0;
        data->auto_scrolling = false;
        return;
      }
    }
}

void UpdateChatFromWebSocket(ChatApp_Data *data) {
  if (!data->ws_data || !data->ws_data->messages) return;
  
  MessageList *msg_list = data->ws_data->messages;
  
  // Store previous message count to detect new messages
  int previous_message_count = chatMessageCount;
  
  // Clear current chat messages
  chatMessageCount = 0;
  
  // Convert WebSocket messages to ChatMessage format
  MessageNode *current = msg_list->head;
  while (current && chatMessageCount < MAX_MESSAGES) {
    Message *ws_msg = &current->message;
    
    // Determine if this message is from current user
    bool isCurrentUser = (strcmp(ws_msg->username, data->login_credentials->username_buf) == 0);
    
    // Create static strings for rendering (need to be persistent)
    static char message_text_storage[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
    static char sender_storage[MAX_MESSAGES][MAX_USERNAME_LENGTH];
    
    strncpy(message_text_storage[chatMessageCount], ws_msg->content, MAX_MESSAGE_LENGTH - 1);
    message_text_storage[chatMessageCount][MAX_MESSAGE_LENGTH - 1] = '\0';
    
    strncpy(sender_storage[chatMessageCount], ws_msg->username, MAX_USERNAME_LENGTH - 1);
    sender_storage[chatMessageCount][MAX_USERNAME_LENGTH - 1] = '\0';
    
    chatMessages[chatMessageCount].text = (Clay_String){
      .chars = message_text_storage[chatMessageCount],
      .length = (int)strlen(message_text_storage[chatMessageCount])
    };
    
    chatMessages[chatMessageCount].sender = (Clay_String){
      .chars = sender_storage[chatMessageCount], 
      .length = (int)strlen(sender_storage[chatMessageCount])
    };
    
    chatMessages[chatMessageCount].isSender = isCurrentUser;
    
    chatMessageCount++;
    current = current->next;
  }
  
  // Auto-scroll to bottom if new messages arrived
  if (chatMessageCount > previous_message_count) {
    // Use placeholder target that will be calculated in UpdateAutoScroll
    data->scroll_target = -999999.0f;
    data->auto_scrolling = true;
    data->scroll_velocity = 0; // Reset velocity for new animation
    data->scroll_delay_frames = 2; // Wait 2 frames for layout to complete
  }
}

Clay_RenderCommandArray ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *data)
{
    data->frameArena.offset = 0;

void HandleSendInteraction(Clay_ElementId elementId,
                           Clay_PointerData pointerData, intptr_t userData) {
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    SendClickData *clickData = (SendClickData *)userData;
    ChatApp_Data *app_data = clickData->app_data;
    
    // Send the message via WebSocket if there's text and connected
    if (app_data->message_len > 0 && app_data->ws_data && app_data->ws_data->connected) {
      // Null-terminate the message buffer
      app_data->message_buffer[app_data->message_len] = '\0';
      
      // Send via WebSocket using the username from login credentials
      websocket_service_send_text(app_data->login_credentials->username_buf, 
                                  app_data->message_buffer);
      
      // Clear the message buffer
      memset(app_data->message_buffer, 0, sizeof(app_data->message_buffer));
      app_data->message_len = 0;
    }
  }
}

void HandleScrollBarInteraction(Clay_ElementId elementId, Clay_PointerData pointerData, intptr_t userData) {
  ScrollBarData *scrollData = (ScrollBarData *)userData;
  ChatApp_Data *data = scrollData->app_data;
  
  if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
    scrollData->is_dragging = true;
    data->auto_scrolling = false; // Stop auto-scroll when user manually scrolls
  } else if (pointerData.state == CLAY_POINTER_DATA_RELEASED_THIS_FRAME) {
    scrollData->is_dragging = false;
  }
  
  if (scrollData->is_dragging && pointerData.state == CLAY_POINTER_DATA_PRESSED) {
    // Get scroll container data
    Clay_ElementId mainContentId = CLAY_ID("MainContent");
    Clay_ScrollContainerData mainScrollData = Clay_GetScrollContainerData(mainContentId);
    
    if (mainScrollData.found && mainScrollData.scrollPosition) {
      // Get the scroll track element to get its position
      Clay_ElementId trackId = CLAY_ID("ScrollBarTrack");
      Clay_ElementData trackElement = Clay_GetElementData(trackId);
      
      if (trackElement.found) {
        // Calculate scroll position based on mouse Y position within scroll track
        float trackTop = trackElement.boundingBox.y + 2; // Account for padding
        float trackHeight = trackElement.boundingBox.height - 4; // Account for padding
        float mouseY = pointerData.position.y - trackTop;
        float scrollRatio = mouseY / trackHeight;
        
        // Clamp ratio between 0 and 1
        if (scrollRatio < 0) scrollRatio = 0;
        if (scrollRatio > 1) scrollRatio = 1;
        
        // Calculate max scroll distance
        float maxScrollY = mainScrollData.contentDimensions.height - mainScrollData.scrollContainerDimensions.height;
        if (maxScrollY > 0) {
          // Directly set the scroll position
          mainScrollData.scrollPosition->y = -scrollRatio * maxScrollY;
        }
      }
    }
  }
}

ChatApp_Data ChatApp_Initialize() {

  ChatApp_Data data = {
    .frameArena = {.memory = (intptr_t)malloc(1024)},
    .auto_scrolling = false,
    .scroll_target = 0,
    .scroll_velocity = 0,
    .scroll_delay_frames = 0,
    .manual_scroll_state = {
      .manual_scroll_velocity = 0,
      .last_manual_scroll_time = 0,
      .is_manual_scrolling = false
    }
  };
  return data;
}

Clay_RenderCommandArray ChatApp_CreateLayout(ChatApp_Data *data) {
  data->frameArena.offset = 0;

  // Update chat messages from WebSocket data
  if (data->ws_data) {
    UpdateChatFromWebSocket(data);
  }
  
  // Update manual scroll velocity
  UpdateManualScrollVelocity(data);
  
  // Update smooth scroll animation
  UpdateAutoScroll(data);

  // Get current window size for responsive fonts
  int windowWidth = GetScreenWidth();   // Raylib function
  int windowHeight = GetScreenHeight(); // Raylib function

  // Calculate responsive font sizes
  int bodyFontSize = GetResponsiveFontSize(24, windowWidth, windowHeight);
  int titleFontSize = GetResponsiveFontSize(28, windowWidth, windowHeight);

  Clay_BeginLayout();

  Clay_Sizing layoutExpand = {.width = CLAY_SIZING_GROW(0),
                              .height = CLAY_SIZING_GROW(0)};

  Clay_Color contentBackgroundColor = {90, 90, 90, 255};

  // Build UI here
  CLAY({.id = CLAY_ID("OuterContainer"),
        .backgroundColor = {43, 41, 51, 255},
        .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing = layoutExpand,
                   .padding = CLAY_PADDING_ALL(16),
                   .childGap = 16}}) {
    // Child elements go inside braces
    CLAY({.id = CLAY_ID("LowerContent"),
          .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                     .sizing = {.width = CLAY_SIZING_GROW(1),
                                .height = CLAY_SIZING_GROW(0)},
                     .childGap = 16}}) {
      CLAY({.id = CLAY_ID("Sidebar"),
            .backgroundColor = contentBackgroundColor,
            .cornerRadius = CLAY_CORNER_RADIUS(8),
            .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                       .padding = CLAY_PADDING_ALL(16),
                       .childGap = 16,
                       .sizing = {.width = CLAY_SIZING_FIT(150),
                                  .height = CLAY_SIZING_GROW(0)}}}) {
        // --- IP Address panel ---
        sprintf((char *)data->login_credentials->ipaddr, "IP Address:\n%s",
                data->login_credentials->ipaddr_buf);
        // check for underscore
        // TODO: place this somewhere else
        size_t ip_len = strlen(data->login_credentials->ipaddr);
        if (data->login_credentials->ipaddr[ip_len - 1] == '_') {
          data->login_credentials->ipaddr[ip_len - 1] = '\0';
          ip_len--;
        }
        Clay_String ipaddr_str =
            (Clay_String){.isStaticallyAllocated = false,
                          .chars = data->login_credentials->ipaddr,
                          .length = ip_len};
        CLAY({.id = CLAY_ID("IpAddress"),
              .backgroundColor = {120, 120, 120, 255},
              .cornerRadius = CLAY_CORNER_RADIUS(8),
              .layout = {.padding = CLAY_PADDING_ALL(12),
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER},
                         .sizing = {.width = CLAY_SIZING_GROW(0)}}}) {
          CLAY_TEXT(ipaddr_str,
                    CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                      .fontSize = bodyFontSize,
                                      .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                                      .textColor = {255, 255, 255, 255}}));
        }

        // --- Username panel ---
        sprintf((char *)data->login_credentials->username, "Username:\n%s",
                data->login_credentials->username_buf);
        // check for underscore
        // TODO: place this somewhere else
        size_t username_len = strlen(data->login_credentials->username);
        if (data->login_credentials->username[username_len - 1] == '_') {
          data->login_credentials->username[username_len - 1] = '\0';
          username_len--;
        }
        Clay_String username_str =
            (Clay_String){.isStaticallyAllocated = false,
                          .chars = data->login_credentials->username,
                          .length = username_len};
        CLAY({.id = CLAY_ID("Username"),
              .backgroundColor = {120, 120, 120, 255},
              .cornerRadius = CLAY_CORNER_RADIUS(8),
              .layout = {.padding = CLAY_PADDING_ALL(12),
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER},
                         .sizing = {.width = CLAY_SIZING_GROW(0)}}}) {
          CLAY_TEXT(username_str,
                    CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                      .fontSize = bodyFontSize,
                                      .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                                      .textColor = {255, 255, 255, 255}}));
        }

        // --- Connection Status panel ---
        Clay_String connection_status;
        Clay_Color status_color;
        
        if (data->ws_data && data->ws_data->connected) {
          connection_status = CLAY_STRING("🟢 Connected");
          status_color = (Clay_Color){0, 150, 0, 255}; // Green
        } else {
          connection_status = CLAY_STRING("🔴 Disconnected");
          status_color = (Clay_Color){150, 0, 0, 255}; // Red
        }
        
        CLAY({.id = CLAY_ID("ConnectionStatus"),
              .backgroundColor = status_color,
              .cornerRadius = CLAY_CORNER_RADIUS(8),
              .layout = {.padding = CLAY_PADDING_ALL(8),
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER},
                         .sizing = {.width = CLAY_SIZING_GROW(0)}}}) {
          CLAY_TEXT(connection_status,
                    CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                      .fontSize = bodyFontSize - 2,
                                      .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                                      .textColor = {255, 255, 255, 255}}));
        }
        // for (int i = 0; i < documents.length; i++) {
        //   Document document = documents.documents[i];
        //   Clay_LayoutConfig sidebarButtonLayout = {
        //       .sizing = {.width = CLAY_SIZING_GROW(0)},
        //       .padding = CLAY_PADDING_ALL(16)};
        //
        //   if (i == data->selectedDocumentIndex) {
        //     CLAY({.layout = sidebarButtonLayout,
        //           .backgroundColor = {120, 120, 120, 255},
        //           .cornerRadius = CLAY_CORNER_RADIUS(8)}) {
        //       CLAY_TEXT(document.title,
        //                 CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
        //                                   .fontSize = titleFontSize,
        //                                   .textColor = {255, 255, 255,
        //                                   255}}));
        //     }
        //   } else {
        //     SidebarClickData *clickData =
        //         (SidebarClickData *)(data->frameArena.memory +
        //                              data->frameArena.offset);
        //     *clickData = (SidebarClickData){.requestedDocumentIndex = i,
        //                                     .selectedDocumentIndex =
        //                                         &data->selectedDocumentIndex};
        //     data->frameArena.offset += sizeof(SidebarClickData);
        //     CLAY({.layout = sidebarButtonLayout,
        //           .backgroundColor =
        //               (Clay_Color){120, 120, 120, Clay_Hovered() ? 120 : 0},
        //           .cornerRadius = CLAY_CORNER_RADIUS(8)}) {
        //       Clay_OnHover(HandleSidebarInteraction, (intptr_t)clickData);
        //       CLAY_TEXT(document.title,
        //                 CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
        //                                   .fontSize = titleFontSize,
        //                                   .textColor = {255, 255, 255,
        //                                   255}}));
        //     }
        //   }
        // }

        // --- Spacer to push logout button down ---
        CLAY({.layout = {.sizing = {.height = CLAY_SIZING_GROW(1)}}});

        // --- Logout button ---
        Clay_Color logoutHover = {120, 120, 120, 255};
        Clay_Color logoutBase = {140, 140, 140, 255};

        CLAY({.id = CLAY_ID("LogoutButton"),
              .backgroundColor =
                  Clay_Hovered() ? logoutHover : logoutBase, // Use hover color
              .cornerRadius = CLAY_CORNER_RADIUS(8),
              .layout = {.padding = CLAY_PADDING_ALL(12),
                         .sizing = {.width = CLAY_SIZING_GROW(0)},
                         .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                            .y = CLAY_ALIGN_Y_CENTER}}}) {
          CLAY_TEXT(CLAY_STRING("Logout"),
                    CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                      .fontSize = bodyFontSize,
                                      .textColor = {255, 255, 255, 255}}));
        }
      }

      // 📌 New vertical container for MainContent + BottomBar
      CLAY({.id = CLAY_ID("RightPane"),
            .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                       .sizing = {.width = CLAY_SIZING_GROW(1),
                                  .height = CLAY_SIZING_GROW(0)},
                       .childGap = 16}}) {
        
        // Main content area with chat and scrollbar  
        CLAY({.id = CLAY_ID("ChatContainer"),
              .backgroundColor = contentBackgroundColor,
              .cornerRadius = CLAY_CORNER_RADIUS(8),
              .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
                         .sizing = {
                             .width = CLAY_SIZING_GROW(1),
                             .height = CLAY_SIZING_GROW(0)},
                         .childGap = 16}})
        {
            CLAY({.id = CLAY_ID("Sidebar"),
                  .backgroundColor = contentBackgroundColor,
                  .cornerRadius = CLAY_CORNER_RADIUS(8),
                  .layout = {
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                      .padding = CLAY_PADDING_ALL(16),
                      .childGap = 16,
                      .sizing = {
                          .width = CLAY_SIZING_FIXED(150),
                          .height = CLAY_SIZING_GROW(0)}}})
            {
                // --- IP Address panel ---
                CLAY({.id = CLAY_ID("IpAddress"),
                      .backgroundColor = {120, 120, 120, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .layout = {
                          .padding = CLAY_PADDING_ALL(12),
                          .sizing = {.width = CLAY_SIZING_GROW(0)}}})
                {
                    CLAY_TEXT(CLAY_STRING("IP Address: 127.0.0.1:1000"),
                              CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = bodyFontSize, .textColor = {255, 255, 255, 255}}));
                }

                // --- Username panel ---
                CLAY({.id = CLAY_ID("Username"),
                      .backgroundColor = {120, 120, 120, 255},
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .layout = {
                          .padding = CLAY_PADDING_ALL(12),
                          .sizing = {.width = CLAY_SIZING_GROW(0)}}})
                {
                    CLAY_TEXT(CLAY_STRING("Username: Ben"),
                              CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = bodyFontSize, .textColor = {255, 255, 255, 255}}));
                }

                // --- Spacer to push logout button down ---
                CLAY({.layout = {.sizing = {.height = CLAY_SIZING_GROW(1)}}});

                // --- Logout button ---
                Clay_Color logoutBase = {120, 120, 120, 255};
                Clay_Color logoutHover = {140, 140, 140, 255};
                Clay_Color logoutColor = logoutBase;
                if (Clay_Hovered())
                    logoutColor = logoutHover;

                CLAY({.id = CLAY_ID("LogoutButton"),
                      .backgroundColor = logoutColor, // Use hover color
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .layout = {
                          .padding = CLAY_PADDING_ALL(12),
                          .sizing = {.width = CLAY_SIZING_GROW(0)},
                          .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}}})
                {
                    CLAY_TEXT(CLAY_STRING("Logout"),
                              CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = bodyFontSize, .textColor = {255, 255, 255, 255}}));
                }
            }

            // 📌 New vertical container for MainContent + BottomBar
            CLAY({.id = CLAY_ID("RightPane"),
                  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                             .sizing = {
                                 .width = CLAY_SIZING_GROW(1),
                                 .height = CLAY_SIZING_GROW(0)},
                             .childGap = 16}})
            {
                CLAY({.id = CLAY_ID("MainContent"),
                      .backgroundColor = contentBackgroundColor,
                      .cornerRadius = CLAY_CORNER_RADIUS(8),
                      .clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
                      .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                                 .childGap = 16,
                                 .padding = CLAY_PADDING_ALL(16),
                                 .sizing = {
                                     .width = CLAY_SIZING_GROW(1),
                                     .height = CLAY_SIZING_GROW(1)}}})

                {
                    for (int i = chatMessageCount - 1; i >= 0; i--)
                    {
                        ChatMessage message = chatMessages[i];

                        bool isUser = message.isSender;

                        CLAY({.layout = {
                                  .layoutDirection = CLAY_LEFT_TO_RIGHT,
                                  .sizing = {.width = CLAY_SIZING_GROW(1)},
                                  .childAlignment = {.x = isUser ? CLAY_ALIGN_X_RIGHT : CLAY_ALIGN_X_LEFT}}})
                        {
                            CLAY({.layout = {
                                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                      .padding = CLAY_PADDING_ALL(10),
                                      .sizing = {.width = CLAY_SIZING_FIT(1)}},
                                  .backgroundColor = isUser ? (Clay_Color){0, 120, 215, 255} : // User bubble (blue)
                                                         (Clay_Color){120, 120, 120, 255},     // Bot bubble (gray)

                                  // Rounded corners like WhatsApp
                                  .cornerRadius = isUser ? (Clay_CornerRadius){.topLeft = 12, .topRight = 12, .bottomLeft = 12, .bottomRight = 2} : (Clay_CornerRadius){.topLeft = 12, .topRight = 12, .bottomLeft = 2, .bottomRight = 12}})
                            {
                                CLAY_TEXT(message.text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                          .fontSize = bodyFontSize,
                                                                          .textColor = {255, 255, 255, 255}}));
                            }
                        }
                    }
                }

                CLAY({.id = CLAY_ID("BottomBar"),
                      .layout = {
                          .layoutDirection = CLAY_LEFT_TO_RIGHT, // Add this line
                          .sizing = {
                              .height = CLAY_SIZING_FIXED(60),
                              .width = CLAY_SIZING_GROW(1)},
                          .padding = CLAY_PADDING_ALL(16), // Change this line
                          .childGap = 16,
                          .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}},
                      .backgroundColor = contentBackgroundColor,
                      .cornerRadius = CLAY_CORNER_RADIUS(8)})
                {
                    CLAY({
                        .id = CLAY_ID("InputBox"),
                        .layout = {.sizing = {
                                       .width = CLAY_SIZING_GROW(1),
                                   }},
                    })
                    {
                        CLAY({.layout = {.padding = {16, 16, 8, 8},
                                         .sizing = {.width = CLAY_SIZING_GROW(0)}},
                              .backgroundColor = {140, 140, 140, 255},
                              .cornerRadius = CLAY_CORNER_RADIUS(5)})
                        {
                            // Add hover colors for input box
                            Clay_Color inputBase = {140, 140, 140, 255};
                            Clay_Color inputHover = {160, 160, 160, 255};
                            Clay_Color inputColor = inputBase;
                            if (Clay_Hovered())
                                inputColor = inputHover;

                            CLAY({.layout = {.padding = CLAY_PADDING_ALL(3),
                                             .sizing = {.width = CLAY_SIZING_GROW(0)}},
                                  .backgroundColor = inputColor,
                                  .cornerRadius = CLAY_CORNER_RADIUS(5)})
                            {
                                CLAY_TEXT(CLAY_STRING("Enter a message.."), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                                              .fontSize = bodyFontSize,
                                                                                              .textColor = {200, 200, 200, 255}}));
                            }
                            // Component_TextBoxData username_data = (Component_TextBoxData) {
                            //    .id = CLAY_STRING("username_textbox"),
                            //    .textConfig = (Clay_TextElementConfig) {
                            //                  .fontId = FONT_ID_BODY_16,
                            //                  .fontSize = 16,
                            //                  .textColor = {255, 255, 255, 255}},
                            //    // .buffer = data->username_buf,
                            //    // .frameCount = &(data->frameCount),
                            //    // .len = &data->username_len,
                            //    // .maxLen = sizeof(data->username_buf),
                            //    // .placeholder = CLAY_STRING("Enter your username:"),
                            //    // .eventData = (TextBoxEventData) {
                            //    //   .focusList = data->focusList,
                            //    //   .isFocus = data->focusList,
                            //    //   .focus_len = 2,
                            //    }
                            //  };

                            // renderTextBox(&username_data);
                        }
                    }
                    Clay_Color base = {140, 140, 140, 255};
                    Clay_Color hover = {160, 160, 160, 255};

                    Clay_Color btnColor = base;
                    if (Clay_Hovered())
                        btnColor = hover;

                    CLAY({.id = CLAY_ID("SendButton"),
                          .layout = {.sizing = {.width = CLAY_SIZING_FIXED(70),
                                                .height = CLAY_SIZING_FIXED(30)},
                                     .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER},
                                     .padding = CLAY_PADDING_ALL(6)},
                          .backgroundColor = btnColor,
                          .cornerRadius = CLAY_CORNER_RADIUS(5)})
                    {
                        SendClickData *clickData = (SendClickData *)(data->frameArena.memory + data->frameArena.offset);
                        *clickData = (SendClickData){0};
                        data->frameArena.offset += sizeof(SendClickData);

                        // Still use OnHover, but inside your handler check if the mouse was clicked
                        Clay_OnHover(HandleSendInteraction, (intptr_t)clickData);

                        CLAY_TEXT(CLAY_STRING("Send"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                                         .fontSize = bodyFontSize,
                                                                         .textColor = {255, 255, 255, 255}}));
                    }
                }

            } // End RightPane
        }
    }

    // if (mouseButtonDown(0) && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ProfilePicture")))) {
    //     // Handle profile picture clicked
    // }
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    for (int32_t i = 0; i < renderCommands.length; i++)
    {
        Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y += data->yOffset;
    }

    return renderCommands;
}
