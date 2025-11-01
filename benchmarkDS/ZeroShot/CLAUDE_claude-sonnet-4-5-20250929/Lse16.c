
#include <stdio.h>
#include <stdbool.h>

#define ARRAY_SIZE 10

static const int ARRAY[ARRAY_SIZE] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

typedef struct {
    int value;
    bool has_value;
} OptionalInt;

OptionalInt getValueAtIndex(int index) {
    OptionalInt result;
    if (index < 0 || index >= ARRAY_SIZE) {
        result.has_value = false;
        result.value = 0;
        return result;
    }
    result.has_value = true;
    result.value = ARRAY[index];
    return result;
}

int main() {
    // Test case 1: Valid index at the beginning
    OptionalInt result1 = getValueAtIndex(0);
    printf("Test 1 (index 0): %s\\n", result1.has_value ? (char[20]){0} : "null");
    if (result1.has_value) printf("Test 1 (index 0): %d\\n", result1.value);
    
    // Test case 2: Valid index in the middle
    OptionalInt result2 = getValueAtIndex(5);
    if (result2.has_value) printf("Test 2 (index 5): %d\\n", result2.value);
    else printf("Test 2 (index 5): null\\n");
    
    // Test case 3: Valid index at the end
    OptionalInt result3 = getValueAtIndex(9);
    if (result3.has_value) printf("Test 3 (index 9): %d\\n", result3.value);
    else printf("Test 3 (index 9): null\\n");
    
    // Test case 4: Invalid negative index
    OptionalInt result4 = getValueAtIndex(-1);
    if (result4.has_value) printf("Test 4 (index -1): %d\\n", result4.value);
    else printf("Test 4 (index -1): null\\n");
    
    // Test case 5: Invalid index beyond array length
    OptionalInt result5 = getValueAtIndex(15);
    if (result5.has_value) printf("Test 5 (index 15): %d\\n", result5.value);
    else printf("Test 5 (index 15): null\\n");
    
    return 0;
}
