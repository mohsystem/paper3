
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool solution(const char* str, const char* ending) {
    if (str == NULL || ending == NULL) {
        return false;
    }
    
    size_t str_len = strlen(str);
    size_t ending_len = strlen(ending);
    
    if (str_len < ending_len) {
        return false;
    }
    
    return strcmp(str + str_len - ending_len, ending) == 0;
}

int main() {
    // Test cases
    printf("%s\\n", solution("abc", "bc") ? "true" : "false");      // true
    printf("%s\\n", solution("abc", "d") ? "true" : "false");       // false
    printf("%s\\n", solution("hello", "lo") ? "true" : "false");    // true
    printf("%s\\n", solution("test", "") ? "true" : "false");       // true
    printf("%s\\n", solution("", "test") ? "true" : "false");       // false
    
    return 0;
}
