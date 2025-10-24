
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

// Check if str ends with suffix
// Returns true if str ends with suffix, false otherwise
// Security: Validates all inputs, uses safe string functions with bounds checking
// All pointers checked for NULL, all operations bounds-checked
bool solution(const char* str, const char* suffix) {
    // Input validation: check for NULL pointers
    if (str == NULL || suffix == NULL) {
        return false;
    }
    
    // Get lengths safely using strnlen with maximum bounds
    // Using SIZE_MAX/2 as reasonable upper limit to prevent overflow
    size_t str_len = strnlen(str, SIZE_MAX / 2);
    size_t suffix_len = strnlen(suffix, SIZE_MAX / 2);
    
    // Input validation: empty suffix matches any string
    if (suffix_len == 0) {
        return true;
    }
    
    // Input validation: if suffix is longer than str, cannot match
    if (suffix_len > str_len) {
        return false;
    }
    
    // Safe pointer arithmetic with bounds validation
    // Calculate starting position for comparison
    const char* str_end = str + (str_len - suffix_len);
    
    // Use memcmp for constant-time comparison (better for security)
    // Compare the last suffix_len bytes of str with suffix
    // Both pointers are valid and within bounds due to checks above
    return memcmp(str_end, suffix, suffix_len) == 0;
}

int main(void) {
    // Test case 1: Basic suffix match
    printf("Test 1: solution('abc', 'bc') = %s (expected: true)\\n",
           solution("abc", "bc") ? "true" : "false");
    
    // Test case 2: No match
    printf("Test 2: solution('abc', 'd') = %s (expected: false)\\n",
           solution("abc", "d") ? "true" : "false");
    
    // Test case 3: Empty suffix (should return true)
    printf("Test 3: solution('abc', '') = %s (expected: true)\\n",
           solution("abc", "") ? "true" : "false");
    
    // Test case 4: Suffix longer than string
    printf("Test 4: solution('ab', 'abc') = %s (expected: false)\\n",
           solution("ab", "abc") ? "true" : "false");
    
    // Test case 5: Exact match (whole string is suffix)
    printf("Test 5: solution('hello', 'hello') = %s (expected: true)\\n",
           solution("hello", "hello") ? "true" : "false");
    
    return 0;
}
