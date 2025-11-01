
#include <iostream>
#include <cstring>
#include <cctype>

char* trimString(const char* input) {
    // Check if input is NULL
    if (input == nullptr) {
        return nullptr;
    }
    
    // Allocate memory for the result
    size_t len = strlen(input);
    char* result = new char[len + 1];
    strcpy(result, input);
    
    // Trim trailing whitespace
    int endIndex = len - 1;
    while (endIndex >= 0 && isspace(result[endIndex])) {
        endIndex--;
    }
    
    // Trim newlines from the end
    while (endIndex >= 0 && (result[endIndex] == '\\n' || result[endIndex] == '\\r')) {
        endIndex--;
    }
    
    // Null terminate the string
    result[endIndex + 1] = '\\0';
    
    return result;
}

int main() {
    // Test cases
    const char* test1 = "hello world   ";
    const char* test2 = "test\\n\\n";
    const char* test3 = "spaces and newlines  \\n\\n  ";
    const char* test4 = "no trailing";
    const char* test5 = "   \\n\\n";
    
    char* result1 = trimString(test1);
    char* result2 = trimString(test2);
    char* result3 = trimString(test3);
    char* result4 = trimString(test4);
    char* result5 = trimString(test5);
    
    std::cout << "Test 1: '" << result1 << "'" << std::endl;
    std::cout << "Test 2: '" << result2 << "'" << std::endl;
    std::cout << "Test 3: '" << result3 << "'" << std::endl;
    std::cout << "Test 4: '" << result4 << "'" << std::endl;
    std::cout << "Test 5: '" << result5 << "'" << std::endl;
    
    delete[] result1;
    delete[] result2;
    delete[] result3;
    delete[] result4;
    delete[] result5;
    
    return 0;
}
