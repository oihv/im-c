#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    bool has_focus;
    char text[256];
    int cursor_position;
    int max_length;
} Clay_TextBox;

bool clay_textbox_handle_input(Clay_TextBox *textbox, char character) {
    if (!textbox || !textbox->has_focus) return false;
    
    if (character == '\b') { /* Backspace */
        if (textbox->cursor_position > 0) {
            int i;
            printf("Before backspace: text='%s' cursor=%d\n", textbox->text, textbox->cursor_position);
            for (i = textbox->cursor_position - 1; i < (int)strlen(textbox->text); i++) {
                textbox->text[i] = textbox->text[i + 1];
            }
            textbox->cursor_position--;
            printf("After backspace: text='%s' cursor=%d\n", textbox->text, textbox->cursor_position);
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

int main() {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 20;
    strcpy(textbox.text, "Testing Message");
    textbox.cursor_position = 7;
    
    printf("Initial: text='%s' cursor=%d\n", textbox.text, textbox.cursor_position);
    
    /* Delete some characters */
    clay_textbox_handle_input(&textbox, '\b');
    clay_textbox_handle_input(&textbox, '\b');
    
    printf("Final: text='%s' cursor=%d\n", textbox.text, textbox.cursor_position);
    printf("Expected text: 'Testi Message', expected cursor: 5\n");
    
    return 0;
}
