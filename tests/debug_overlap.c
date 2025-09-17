#include <stdio.h>
#include <stdbool.h>

typedef struct {
    float x, y, width, height;
} Clay_BoundingBox;

bool clay_layout_elements_overlap(Clay_BoundingBox box1, Clay_BoundingBox box2) {
    printf("Box1: x=%.1f, y=%.1f, w=%.1f, h=%.1f (ends at x=%.1f, y=%.1f)\n", 
           box1.x, box1.y, box1.width, box1.height, box1.x + box1.width, box1.y + box1.height);
    printf("Box2: x=%.1f, y=%.1f, w=%.1f, h=%.1f (ends at x=%.1f, y=%.1f)\n", 
           box2.x, box2.y, box2.width, box2.height, box2.x + box2.width, box2.y + box2.height);
    
    bool cond1 = box1.x + box1.width <= box2.x;
    bool cond2 = box2.x + box2.width <= box1.x;
    bool cond3 = box1.y + box1.height <= box2.y;
    bool cond4 = box2.y + box2.height <= box1.y;
    
    printf("Conditions: cond1=%d, cond2=%d, cond3=%d, cond4=%d\n", cond1, cond2, cond3, cond4);
    printf("Any condition true? %d\n", cond1 || cond2 || cond3 || cond4);
    
    bool result = !(cond1 || cond2 || cond3 || cond4);
    printf("Overlap result: %d\n", result);
    return result;
}

int main() {
    Clay_BoundingBox box1 = {10, 10, 50, 50}; /* Ends at 60, 60 */
    Clay_BoundingBox box2 = {60, 60, 50, 50}; /* Starts at 60, 60 */
    
    bool overlap = clay_layout_elements_overlap(box1, box2);
    printf("\nExpected: 0 (no overlap), Got: %d\n", overlap);
    return 0;
}
