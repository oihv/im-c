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
            
            printf("Before insert '%c' at pos %d: text='%s' len=%d\n", 
                   character, textbox->cursor_position, textbox->text, len);
            
            /* Shift characters to the right */
            for (i = len; i >= textbox->cursor_position; i--) {
                textbox->text[i + 1] = textbox->text[i];
            }
            
            textbox->text[textbox->cursor_position] = character;
            textbox->cursor_position++;
            
            printf("After insert: text='%s' cursor=%d\n", 
                   textbox->text, textbox->cursor_position);
            return true;
        }
    }
    
    return false;
}

int main() {
    Clay_TextBox textbox = {0};
    textbox.has_focus = true;
    textbox.max_length = 20;
    strcpy(textbox.text, "Test");
    textbox.cursor_position = 4;
    
    printf("Initial: text='%s' cursor=%d\n", textbox.text, textbox.cursor_position);
    
    /* Add " Message" */
    clay_textbox_handle_input(&textbox, ' ');
    clay_textbox_handle_input(&textbox, 'M');
    clay_textbox_handle_input(&textbox, 'e');
    clay_textbox_handle_input(&textbox, 's');
    clay_textbox_handle_input(&textbox, 's');
    clay_textbox_handle_input(&textbox, 'a');
    clay_textbox_handle_input(&textbox, 'g');
    clay_textbox_handle_input(&textbox, 'e');
    
    printf("After adding ' Message': text='%s' cursor=%d\n", textbox.text, textbox.cursor_position);
    
    /* Move cursor to middle and insert text */
    textbox.cursor_position = 5;
    printf("Moved cursor to position 5\n");
    clay_textbox_handle_input(&textbox, 'i');
    clay_textbox_handle_input(&textbox, 'n');
    clay_textbox_handle_input(&textbox, 'g');
    
    printf("Final result: text='%s' cursor=%d\n", textbox.text, textbox.cursor_position);
    printf("Expected: 'Testing Message'\n");
    
    return 0;
}
