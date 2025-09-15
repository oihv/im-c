#include "unity.h"
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// Mock structures for UI testing
typedef struct {
    float x, y, width, height;
} BoundingBox;

// Test helper functions
bool isPointInBoundingBox(float x, float y, BoundingBox box) {
    return x >= box.x && x <= box.x + box.width &&
           y >= box.y && y <= box.y + box.height;
}

void handleTextInput(char* buffer, size_t* len, size_t maxLen, char inputChar) {
    if (!buffer || !len) return;
    
    if (*len < maxLen - 1) {
        buffer[*len] = inputChar;
        (*len)++;
        buffer[*len] = '\0';
    }
}

void handleBackspace(char* buffer, size_t* len) {
    if (!buffer || !len) return;
    
    if (*len > 0) {
        (*len)--;
        buffer[*len] = '\0';
    }
}

void setUp(void) {
}

void tearDown(void) {
}

void test_isPointInBoundingBox_inside(void) {
    BoundingBox box = {10.0f, 10.0f, 100.0f, 50.0f};
    
    TEST_ASSERT_TRUE(isPointInBoundingBox(50.0f, 30.0f, box));
    TEST_ASSERT_TRUE(isPointInBoundingBox(10.0f, 10.0f, box));
    TEST_ASSERT_TRUE(isPointInBoundingBox(110.0f, 60.0f, box));
}

void test_isPointInBoundingBox_outside(void) {
    BoundingBox box = {10.0f, 10.0f, 100.0f, 50.0f};
    
    TEST_ASSERT_FALSE(isPointInBoundingBox(5.0f, 30.0f, box));
    TEST_ASSERT_FALSE(isPointInBoundingBox(50.0f, 5.0f, box));
    TEST_ASSERT_FALSE(isPointInBoundingBox(115.0f, 30.0f, box));
    TEST_ASSERT_FALSE(isPointInBoundingBox(50.0f, 65.0f, box));
}

void test_handleTextInput_normal(void) {
    size_t len = 0;
    char buffer[10] = "";
    
    handleTextInput(buffer, &len, 10, 'H');
    handleTextInput(buffer, &len, 10, 'i');
    
    TEST_ASSERT_EQUAL_STRING("Hi", buffer);
    TEST_ASSERT_EQUAL_INT(2, len);
}

void test_handleTextInput_buffer_full(void) {
    size_t len = 3;
    char buffer[5] = "abc";
    
    handleTextInput(buffer, &len, 5, 'd');
    handleTextInput(buffer, &len, 5, 'e'); // Should be ignored
    
    TEST_ASSERT_EQUAL_STRING("abcd", buffer);
    TEST_ASSERT_EQUAL_INT(4, len);
}

void test_handleBackspace_normal(void) {
    size_t len = 5;
    char buffer[10] = "Hello";
    
    handleBackspace(buffer, &len);
    
    TEST_ASSERT_EQUAL_STRING("Hell", buffer);
    TEST_ASSERT_EQUAL_INT(4, len);
}

void test_handleBackspace_empty_buffer(void) {
    size_t len = 0;
    char buffer[10] = "";
    
    handleBackspace(buffer, &len);
    
    TEST_ASSERT_EQUAL_STRING("", buffer);
    TEST_ASSERT_EQUAL_INT(0, len);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_isPointInBoundingBox_inside);
    RUN_TEST(test_isPointInBoundingBox_outside);
    RUN_TEST(test_handleTextInput_normal);
    RUN_TEST(test_handleTextInput_buffer_full);
    RUN_TEST(test_handleBackspace_normal);
    RUN_TEST(test_handleBackspace_empty_buffer);
    
    return UNITY_END();
}