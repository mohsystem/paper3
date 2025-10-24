#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool solution(const char* str, const char* ending) {
    if (str == NULL || ending == NULL) {
        return false;
    }
    
    size_t str_len = strlen(str);
    size_t ending_len = strlen(ending);

    if (ending_len > str_len) {
        return false;
    }

    // Compare the end of the string 'str' with 'ending'
    return strcmp(str + (str_len - ending_len), ending) == 0;
}

int main() {
    // Helper to print boolean values as strings
    #define print_bool(b) printf("%s\n", (b) ? "true" : "false")

    // Test cases
    printf("Test Case 1: solution('abc', 'bc')\n");
    printf("Result: ");
    print_bool(solution("abc", "bc")); // Expected: true

    printf("\nTest Case 2: solution('abc', 'd')\n");
    printf("Result: ");
    print_bool(solution("abc", "d")); // Expected: false

    printf("\nTest Case 3: solution('samurai', 'ai')\n");
    printf("Result: ");
    print_bool(solution("samurai", "ai")); // Expected: true
    
    printf("\nTest Case 4: solution('abc', '')\n");
    printf("Result: ");
    print_bool(solution("abc", "")); // Expected: true
    
    printf("\nTest Case 5: solution('test', 'testing')\n");
    printf("Result: ");
    print_bool(solution("test", "testing")); // Expected: false

    return 0;
}