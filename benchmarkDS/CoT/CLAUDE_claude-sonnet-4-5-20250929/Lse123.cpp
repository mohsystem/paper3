
#include <iostream>
#include <cstring>
#include <cctype>

class Lse123 {
public:
    static void processString(int argc, char* argv[], char* result, size_t maxLen) {
        // Check if exactly one argument is provided
        if (argc != 2) {
            strncpy(result, "Error: Exactly one argument required", maxLen - 1);
            result[maxLen - 1] = '\\0';
            return;
        }
        
        // Check if the argument is valid
        if (argv[1] == nullptr) {
            result[0] = '\\0';
            return;
        }
        
        // Safely copy input to result buffer
        size_t len = strlen(argv[1]);
        if (len >= maxLen) {
            len = maxLen - 1;
        }
        strncpy(result, argv[1], len);
        result[len] = '\\0';
        
        // Trim trailing whitespace
        int end = len - 1;
        while (end >= 0 && isspace(static_cast<unsigned char>(result[end]))) {
            end--;
        }
        
        // Trim newlines
        while (end >= 0 && (result[end] == '\\n' || result[end] == '\\r')) {
            end--;
        }
        
        // Null terminate the string
        result[end + 1] = '\\0';
    }
};

int main() {
    const size_t bufSize = 256;
    char result[bufSize];
    
    // Test case 1: Normal string with trailing spaces
    char* test1[] = {(char*)"program", (char*)"Hello World   "};
    Lse123::processString(2, test1, result, bufSize);
    std::cout << "Test 1: '" << result << "'" << std::endl;
    
    // Test case 2: String with trailing newlines
    char* test2[] = {(char*)"program", (char*)"Hello\\n\\n"};
    Lse123::processString(2, test2, result, bufSize);
    std::cout << "Test 2: '" << result << "'" << std::endl;
    
    // Test case 3: String with both spaces and newlines
    char* test3[] = {(char*)"program", (char*)"Test String  \\n  \\n"};
    Lse123::processString(2, test3, result, bufSize);
    std::cout << "Test 3: '" << result << "'" << std::endl;
    
    // Test case 4: Empty string
    char* test4[] = {(char*)"program", (char*)""};
    Lse123::processString(2, test4, result, bufSize);
    std::cout << "Test 4: '" << result << "'" << std::endl;
    
    // Test case 5: No trailing whitespace
    char* test5[] = {(char*)"program", (char*)"Clean"};
    Lse123::processString(2, test5, result, bufSize);
    std::cout << "Test 5: '" << result << "'" << std::endl;
    
    return 0;
}
