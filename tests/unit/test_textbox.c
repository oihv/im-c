#include "unity.h"
#include <stdbool.h>
#include <string.h>

// Simple test functions for textbox logic without dependencies
void resetTextBoxFocus(bool* focusList, size_t len) {
    if (!focusList) return;
    for (size_t i = 0; i < len; i++) {
        focusList[i] = false;
    }
}

void setUp(void) {
}

void tearDown(void) {
}

void test_resetTextBoxFocus_all_false(void) {
    bool focusList[5] = {true, true, false, true, false};
    
    resetTextBoxFocus(focusList, 5);
    
    for (int i = 0; i < 5; i++) {
        TEST_ASSERT_FALSE(focusList[i]);
    }
}

void test_resetTextBoxFocus_empty_list(void) {
    bool* focusList = NULL;
    
    resetTextBoxFocus(focusList, 0);
    
    // No assertion needed - test passes if it doesn't crash
}

void test_resetTextBoxFocus_single_element(void) {
    bool focusList[1] = {true};
    
    resetTextBoxFocus(focusList, 1);
    
    TEST_ASSERT_FALSE(focusList[0]);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_resetTextBoxFocus_all_false);
    RUN_TEST(test_resetTextBoxFocus_empty_list);
    RUN_TEST(test_resetTextBoxFocus_single_element);
    
    return UNITY_END();
}