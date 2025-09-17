#include "unity.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Mock Clay structures for headless testing */
typedef struct {
    float x, y, width, height;
} Clay_BoundingBox;

typedef struct {
    float x, y;
} Clay_Vector2;

typedef struct {
    bool has_focus;
    char text[256];
    int cursor_position;
    int max_length;
} Clay_TextBox;

typedef struct {
    Clay_BoundingBox bounding_box;
    bool is_hovered;
    bool is_pressed;
    bool is_clicked;
} Clay_Button;

typedef enum {
    CLAY_COLOR_RED = 0xFF0000,
    CLAY_COLOR_GREEN = 0x00FF00,
    CLAY_COLOR_BLUE = 0x0000FF,
    CLAY_COLOR_WHITE = 0xFFFFFF,
    CLAY_COLOR_BLACK = 0x000000
} Clay_Color;

/* UI component functions */
bool clay_point_in_bounds(Clay_Vector2 point, Clay_BoundingBox bounds) {
    return point.x >= bounds.x && 
           point.x <= bounds.x + bounds.width &&
           point.y >= bounds.y && 
           point.y <= bounds.y + bounds.height;
}

bool clay_textbox_handle_input(Clay_TextBox *textbox, char character) {
    if (!textbox || !textbox->has_focus) return false;
    
    if (character == '\b') { /* Backspace */
        if (textbox->cursor_position > 0) {
            int i;
            for (i = textbox->cursor_position - 1; i < (int)strlen(textbox->text); i++) {
                textbox->text[i] = textbox->text[i + 1];
            }
            textbox->cursor_position--;
            return true;
        }
    } else if (character >= 32 && character <= 126) { /* Printable ASCII */
        int len = strlen(textbox->text);
        if (len < textbox->max_length - 1) {
            int i;
            
            /* Shift characters to the right */
            for (i = len; i >= textbox->cursor_position; i--) {
                textbox->text[i + 1] = textbox->text[i];
            }
            
            textbox->text[textbox->cursor_position] = character;
            textbox->cursor_position++;
            return true;
        }
    }
    
    return false;
}

void clay_textbox_set_focus(Clay_TextBox *textbox, bool focus) {
    if (textbox) {
        textbox->has_focus = focus;
    }
}

bool clay_button_handle_click(Clay_Button *button, Clay_Vector2 click_position) {
    if (!button) return false;
    
    if (clay_point_in_bounds(click_position, button->bounding_box)) {
        button->is_clicked = true;
        return true;
    }
    
    return false;
}

void clay_button_update_hover(Clay_Button *button, Clay_Vector2 mouse_position) {
    if (!button) return;
    
    button->is_hovered = clay_point_in_bounds(mouse_position, button->bounding_box);
}

/* Layout testing functions */
bool clay_layout_elements_overlap(Clay_BoundingBox box1, Clay_BoundingBox box2) {
    return !(box1.x + box1.width <= box2.x || 
             box2.x + box2.width <= box1.x ||
             box1.y + box1.height <= box2.y || 
             box2.y + box2.height <= box1.y);
}

float clay_layout_calculate_total_width(Clay_BoundingBox *elements, int count) {
    float total_width = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        float right_edge = elements[i].x + elements[i].width;
        if (right_edge > total_width) {
            total_width = right_edge;
        }
    }
    
    return total_width;
}

/* Test cases */
void setUp(void) {
    /* Setup before each test */
}

void tearDown(void) {
    /* Cleanup after each test */
}

void test_clay_point_in_bounds_inside(void) {
    Clay_BoundingBox box = {10, 20, 100, 50};
    Clay_Vector2 point = {50, 40};
    
    TEST_ASSERT_TRUE(clay_point_in_bounds(point, box));
}

void test_clay_point_in_bounds_outside(void) {
    Clay_BoundingBox box = {10, 20, 100, 50};
    Clay_Vector2 point = {5, 40};
    
    TEST_ASSERT_FALSE(clay_point_in_bounds(point, box));
}

void test_clay_point_in_bounds_on_edge(void) {
    Clay_BoundingBox box = {10, 20, 100, 50};
    Clay_Vector2 point1 = {10, 40}; /* Left edge */
    Clay_Vector2 point2 = {110, 40}; /* Right edge */
    Clay_Vector2 point3 = {50, 20}; /* Top edge */
    Clay_Vector2 point4 = {50, 70}; /* Bottom edge */
    
    TEST_ASSERT_TRUE(clay_point_in_bounds(point1, box));
    TEST_ASSERT_TRUE(clay_point_in_bounds(point2, box));
    TEST_ASSERT_TRUE(clay_point_in_bounds(point3, box));
    TEST_ASSERT_TRUE(clay_point_in_bounds(point4, box));
}

void test_clay_textbox_input_normal_character(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 256;
    strcpy(textbox.text, "Hello");
    textbox.cursor_position = 5;
    
    TEST_ASSERT_TRUE(clay_textbox_handle_input(&textbox, ' '));
    TEST_ASSERT_EQUAL_STRING("Hello ", textbox.text);
    TEST_ASSERT_EQUAL_INT(6, textbox.cursor_position);
}

void test_clay_textbox_input_insertion(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 256;
    strcpy(textbox.text, "Hello World");
    textbox.cursor_position = 5; /* Between "Hello" and " World" */
    
    TEST_ASSERT_TRUE(clay_textbox_handle_input(&textbox, '!'));
    TEST_ASSERT_EQUAL_STRING("Hello! World", textbox.text);
    TEST_ASSERT_EQUAL_INT(6, textbox.cursor_position);
}

void test_clay_textbox_backspace_middle(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 256;
    strcpy(textbox.text, "Hello World");
    textbox.cursor_position = 5; /* After "Hello" */
    
    TEST_ASSERT_TRUE(clay_textbox_handle_input(&textbox, '\b'));
    TEST_ASSERT_EQUAL_STRING("Hell World", textbox.text);
    TEST_ASSERT_EQUAL_INT(4, textbox.cursor_position);
}

void test_clay_textbox_backspace_at_start(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 256;
    strcpy(textbox.text, "Hello");
    textbox.cursor_position = 0;
    
    TEST_ASSERT_FALSE(clay_textbox_handle_input(&textbox, '\b'));
    TEST_ASSERT_EQUAL_STRING("Hello", textbox.text);
    TEST_ASSERT_EQUAL_INT(0, textbox.cursor_position);
}

void test_clay_textbox_max_length_limit(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 6; /* Small limit for testing */
    strcpy(textbox.text, "Hello");
    textbox.cursor_position = 5;
    
    TEST_ASSERT_FALSE(clay_textbox_handle_input(&textbox, '!'));
    TEST_ASSERT_EQUAL_STRING("Hello", textbox.text);
    TEST_ASSERT_EQUAL_INT(5, textbox.cursor_position);
}

void test_clay_textbox_no_focus(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = false;
    textbox.max_length = 256;
    strcpy(textbox.text, "Hello");
    textbox.cursor_position = 5;
    
    TEST_ASSERT_FALSE(clay_textbox_handle_input(&textbox, '!'));
    TEST_ASSERT_EQUAL_STRING("Hello", textbox.text);
    TEST_ASSERT_EQUAL_INT(5, textbox.cursor_position);
}

void test_clay_textbox_special_characters(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 256;
    strcpy(textbox.text, "");
    textbox.cursor_position = 0;
    
    /* Test printable special characters */
    TEST_ASSERT_TRUE(clay_textbox_handle_input(&textbox, '@'));
    TEST_ASSERT_TRUE(clay_textbox_handle_input(&textbox, '#'));
    TEST_ASSERT_TRUE(clay_textbox_handle_input(&textbox, '$'));
    
    TEST_ASSERT_EQUAL_STRING("@#$", textbox.text);
    TEST_ASSERT_EQUAL_INT(3, textbox.cursor_position);
    
    /* Test non-printable characters (should be ignored) */
    TEST_ASSERT_FALSE(clay_textbox_handle_input(&textbox, '\n'));
    TEST_ASSERT_FALSE(clay_textbox_handle_input(&textbox, '\t'));
    TEST_ASSERT_FALSE(clay_textbox_handle_input(&textbox, 1)); /* Control character */
    
    TEST_ASSERT_EQUAL_STRING("@#$", textbox.text);
    TEST_ASSERT_EQUAL_INT(3, textbox.cursor_position);
}

void test_clay_button_click_inside(void) {
    Clay_Button button = {0};
    button.bounding_box = (Clay_BoundingBox){50, 50, 100, 30};
    Clay_Vector2 click_pos = {75, 60};
    
    TEST_ASSERT_TRUE(clay_button_handle_click(&button, click_pos));
    TEST_ASSERT_TRUE(button.is_clicked);
}

void test_clay_button_click_outside(void) {
    Clay_Button button = {0};
    button.bounding_box = (Clay_BoundingBox){50, 50, 100, 30};
    Clay_Vector2 click_pos = {40, 60};
    
    TEST_ASSERT_FALSE(clay_button_handle_click(&button, click_pos));
    TEST_ASSERT_FALSE(button.is_clicked);
}

void test_clay_button_hover_detection(void) {
    Clay_Button button = {0};
    button.bounding_box = (Clay_BoundingBox){50, 50, 100, 30};
    Clay_Vector2 mouse_inside = {75, 60};
    Clay_Vector2 mouse_outside = {40, 60};
    
    /* Test hover inside */
    clay_button_update_hover(&button, mouse_inside);
    TEST_ASSERT_TRUE(button.is_hovered);
    
    /* Test hover outside */
    clay_button_update_hover(&button, mouse_outside);
    TEST_ASSERT_FALSE(button.is_hovered);
}

void test_clay_layout_overlap_detection(void) {
    Clay_BoundingBox box1 = {10, 10, 50, 50};
    Clay_BoundingBox box2 = {40, 40, 50, 50}; /* Overlaps */
    Clay_BoundingBox box3 = {70, 70, 50, 50}; /* No overlap */
    
    TEST_ASSERT_TRUE(clay_layout_elements_overlap(box1, box2));
    TEST_ASSERT_FALSE(clay_layout_elements_overlap(box1, box3));
}

void test_clay_layout_no_overlap_edge_case(void) {
    Clay_BoundingBox box1 = {10, 10, 50, 50}; /* Ends at 60, 60 */
    Clay_BoundingBox box2 = {60, 60, 50, 50}; /* Starts at 60, 60 */
    
    /* Touching edges should not be considered overlap */
    TEST_ASSERT_FALSE(clay_layout_elements_overlap(box1, box2));
}

void test_clay_layout_total_width_calculation(void) {
    Clay_BoundingBox elements[3] = {
        {10, 10, 50, 30},  /* Ends at x=60 */
        {70, 10, 40, 30},  /* Ends at x=110 */
        {20, 50, 30, 30}   /* Ends at x=50 */
    };
    
    float total_width = clay_layout_calculate_total_width(elements, 3);
    TEST_ASSERT_EQUAL_FLOAT(110.0f, total_width);
}

void test_clay_textbox_focus_management(void) {
    Clay_TextBox textbox1 = {0};
    Clay_TextBox textbox2 = {0};
    
    /* Test setting focus */
    clay_textbox_set_focus(&textbox1, true);
    TEST_ASSERT_TRUE(textbox1.has_focus);
    
    /* Test removing focus */
    clay_textbox_set_focus(&textbox1, false);
    TEST_ASSERT_FALSE(textbox1.has_focus);
    
    /* Test multiple textboxes focus management */
    clay_textbox_set_focus(&textbox1, true);
    clay_textbox_set_focus(&textbox2, true);
    
    /* Both can have focus simultaneously (application logic should handle exclusivity) */
    TEST_ASSERT_TRUE(textbox1.has_focus);
    TEST_ASSERT_TRUE(textbox2.has_focus);
}

void test_clay_ui_component_null_safety(void) {
    Clay_Vector2 point = {50, 50};
    
    /* Test null safety for all functions */
    TEST_ASSERT_FALSE(clay_textbox_handle_input(NULL, 'a'));
    TEST_ASSERT_FALSE(clay_button_handle_click(NULL, point));
    
    /* These should not crash */
    clay_textbox_set_focus(NULL, true);
    clay_button_update_hover(NULL, point);
}

void test_clay_complex_textbox_scenario(void) {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 20;
    strcpy(textbox.text, "Test");
    textbox.cursor_position = 4;
    
    /* Add " Message" */
    clay_textbox_handle_input(&textbox, ' ');
    clay_textbox_handle_input(&textbox, 'M');
    clay_textbox_handle_input(&textbox, 'e');
    clay_textbox_handle_input(&textbox, 's');
    clay_textbox_handle_input(&textbox, 's');
    clay_textbox_handle_input(&textbox, 'a');
    clay_textbox_handle_input(&textbox, 'g');
    clay_textbox_handle_input(&textbox, 'e');
    
    TEST_ASSERT_EQUAL_STRING("Test Message", textbox.text);
    TEST_ASSERT_EQUAL_INT(12, textbox.cursor_position);
    
    /* Move cursor to middle and insert text */
    textbox.cursor_position = 4;
    clay_textbox_handle_input(&textbox, 'i');
    clay_textbox_handle_input(&textbox, 'n');
    clay_textbox_handle_input(&textbox, 'g');
    
    TEST_ASSERT_EQUAL_STRING("Testing Message", textbox.text);
    TEST_ASSERT_EQUAL_INT(7, textbox.cursor_position);
    
    /* Delete some characters */
    clay_textbox_handle_input(&textbox, '\b');
    clay_textbox_handle_input(&textbox, '\b');
    
    TEST_ASSERT_EQUAL_STRING("Testi Message", textbox.text);
    TEST_ASSERT_EQUAL_INT(5, textbox.cursor_position);
}

int main(void) {
    UNITY_BEGIN();
    
    /* Point in bounds tests */
    RUN_TEST(test_clay_point_in_bounds_inside);
    RUN_TEST(test_clay_point_in_bounds_outside);
    RUN_TEST(test_clay_point_in_bounds_on_edge);
    
    /* Textbox input tests */
    RUN_TEST(test_clay_textbox_input_normal_character);
    RUN_TEST(test_clay_textbox_input_insertion);
    RUN_TEST(test_clay_textbox_backspace_middle);
    RUN_TEST(test_clay_textbox_backspace_at_start);
    RUN_TEST(test_clay_textbox_max_length_limit);
    RUN_TEST(test_clay_textbox_no_focus);
    RUN_TEST(test_clay_textbox_special_characters);
    
    /* Button interaction tests */
    RUN_TEST(test_clay_button_click_inside);
    RUN_TEST(test_clay_button_click_outside);
    RUN_TEST(test_clay_button_hover_detection);
    
    /* Layout tests */
    RUN_TEST(test_clay_layout_overlap_detection);
    RUN_TEST(test_clay_layout_no_overlap_edge_case);
    RUN_TEST(test_clay_layout_total_width_calculation);
    
    /* Focus management tests */
    RUN_TEST(test_clay_textbox_focus_management);
    
    /* Error handling tests */
    RUN_TEST(test_clay_ui_component_null_safety);
    
    /* Complex scenario tests */
    RUN_TEST(test_clay_complex_textbox_scenario);
    
    return UNITY_END();
}