
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool solution(const char* str, const char* ending) {
    // Input validation to prevent null pointer dereference
    if (str == NULL || ending == NULL) {
        return false;
    }
    
    size_t str_len = strlen(str);
    size_t ending_len = strlen(ending);
    
    // Check if ending is longer than the string
    if (ending_len > str_len) {
        return false;
    }
    
    // Compare the end of str with ending using safe comparison
    // Use strncmp to prevent buffer overflow
    return strncmp(str + (str_len - ending_len), ending, ending_len) == 0;
}

int main() {
    // Test case 1: Basic positive case
    printf("%s\\n", solution("abc", "bc") ? "true" : "false"); // true
    
    // Test case 2: Basic negative case
    printf("%s\\n", solution("abc", "d") ? "true" : "false"); // false
    
    // Test case 3: Empty ending (edge case)
    printf("%s\\n", solution("abc", "") ? "true" : "false"); // true
    
    // Test case 4: Ending longer than string
    printf("%s\\n", solution("ab", "abc") ? "true" : "false"); // false
    
    // Test case 5: Null safety test
    printf("%s\\n", solution(NULL, "bc") ? "true" : "false"); // false
    
    return 0;
}
