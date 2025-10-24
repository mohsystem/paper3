
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool solution(const char* str, const char* ending) {
    int str_len = strlen(str);
    int ending_len = strlen(ending);
    
    if (ending_len > str_len) {
        return false;
    }
    
    return strcmp(str + (str_len - ending_len), ending) == 0;
}

int main() {
    // Test case 1
    printf("%s\\n", solution("abc", "bc") ? "true" : "false"); // true
    
    // Test case 2
    printf("%s\\n", solution("abc", "d") ? "true" : "false"); // false
    
    // Test case 3
    printf("%s\\n", solution("hello world", "world") ? "true" : "false"); // true
    
    // Test case 4
    printf("%s\\n", solution("test", "") ? "true" : "false"); // true
    
    // Test case 5
    printf("%s\\n", solution("programming", "ing") ? "true" : "false"); // true
    
    return 0;
}
