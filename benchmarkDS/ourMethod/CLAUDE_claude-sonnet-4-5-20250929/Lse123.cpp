
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cctype>

// Process string by trimming trailing whitespace and newlines
// Returns new allocated string that must be freed by caller
char* process_string(const char* input) {
    // Validate input is not NULL
    if (input == NULL) {
        return NULL;
    }
    
    // Validate length to prevent excessive memory usage
    const size_t MAX_LENGTH = 1048576; // 1MB limit
    size_t input_len = strlen(input);
    if (input_len > MAX_LENGTH) {
        std::cerr << "Error: Input exceeds maximum allowed length" << std::endl;
        return NULL;
    }
    
    // Allocate buffer with bounds check
    char* result = static_cast<char*>(malloc(input_len + 1));
    if (result == NULL) {
        std::cerr << "Error: Memory allocation failed" << std::endl;
        return NULL;
    }
    
    // Copy input to result with bounds checking
    strncpy(result, input, input_len);
    result[input_len] = '\\0'; // Ensure null termination
    
    size_t length = input_len;
    
    // First loop: trim trailing whitespace
    // Iterate from end to find last non-whitespace character
    while (length > 0 && isspace(static_cast<unsigned char>(result[length - 1]))) {
        length--;
    }
    
    // Second loop: trim trailing newlines (redundant but follows specification)
    while (length > 0 && (result[length - 1] == '\\n' || result[length - 1] == '\\r')) {
        length--;
    }
    
    // Null terminate the string at new length
    result[length] = '\\0';
    
    return result;
}

int main(int argc, char* argv[]) {
    // Check that exactly one argument is provided
    if (argc != 2) {
        std::cerr << "Error: Exactly one argument required" << std::endl;
        return 1;
    }
    
    // Check the argument is not NULL (defensive check)
    if (argv[1] == NULL) {
        std::cerr << "Error: Argument must be a valid string" << std::endl;
        return 1;
    }
    
    // Process the string
    char* result = process_string(argv[1]);
    if (result == NULL) {
        std::cerr << "Error: Failed to process string" << std::endl;
        return 1;
    }
    
    std::cout << "Result: '" << result << "'" << std::endl;
    free(result);
    result = NULL;
    
    // Test cases
    std::cout << "\\n=== Test Cases ===" << std::endl;
    
    // Test 1: String with trailing spaces
    const char* test1 = "hello world   ";
    std::cout << "Test 1 Input: '" << test1 << "'" << std::endl;
    char* out1 = process_string(test1);
    if (out1) {
        std::cout << "Test 1 Output: '" << out1 << "'" << std::endl;
        free(out1);
    }
    
    // Test 2: String with trailing newlines
    const char* test2 = "hello world\\n\\n";
    std::cout << "Test 2 Input: 'hello world\\\\n\\\\n'" << std::endl;
    char* out2 = process_string(test2);
    if (out2) {
        std::cout << "Test 2 Output: '" << out2 << "'" << std::endl;
        free(out2);
    }
    
    // Test 3: String with mixed trailing whitespace
    const char* test3 = "hello world \\n \\t ";
    std::cout << "Test 3 Input: 'hello world \\\\n \\\\t '" << std::endl;
    char* out3 = process_string(test3);
    if (out3) {
        std::cout << "Test 3 Output: '" << out3 << "'" << std::endl;
        free(out3);
    }
    
    // Test 4: String with no trailing whitespace
    const char* test4 = "hello world";
    std::cout << "Test 4 Input: '" << test4 << "'" << std::endl;
    char* out4 = process_string(test4);
    if (out4) {
        std::cout << "Test 4 Output: '" << out4 << "'" << std::endl;
        free(out4);
    }
    
    // Test 5: Empty string
    const char* test5 = "";
    std::cout << "Test 5 Input: ''" << std::endl;
    char* out5 = process_string(test5);
    if (out5) {
        std::cout << "Test 5 Output: '" << out5 << "'" << std::endl;
        free(out5);
    }
    
    return 0;
}
