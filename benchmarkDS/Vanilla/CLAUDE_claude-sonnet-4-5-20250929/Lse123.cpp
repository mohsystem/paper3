
#include <iostream>
#include <cstring>
#include <cctype>

using namespace std;

char* processString(const char* input) {
    if (input == nullptr) {
        return nullptr;
    }
    
    int len = strlen(input);
    char* result = new char[len + 1];
    strcpy(result, input);
    
    // Trim trailing whitespace
    int end = len - 1;
    while (end >= 0 && isspace(result[end])) {
        end--;
    }
    
    // Trim newlines
    while (end >= 0 && (result[end] == '\\n' || result[end] == '\\r')) {
        end--;
    }
    
    // Null terminate the string
    result[end + 1] = '\\0';
    
    return result;
}

int main() {
    // Test case 1: String with trailing spaces
    char* result1 = processString("hello world   ");
    cout << "Test 1: '" << result1 << "'" << endl;
    delete[] result1;
    
    // Test case 2: String with trailing newlines
    char* result2 = processString("hello world\\n\\n");
    cout << "Test 2: '" << result2 << "'" << endl;
    delete[] result2;
    
    // Test case 3: String with trailing spaces and newlines
    char* result3 = processString("hello world  \\n\\r ");
    cout << "Test 3: '" << result3 << "'" << endl;
    delete[] result3;
    
    // Test case 4: String with no trailing whitespace
    char* result4 = processString("hello world");
    cout << "Test 4: '" << result4 << "'" << endl;
    delete[] result4;
    
    // Test case 5: Empty string
    char* result5 = processString("");
    cout << "Test 5: '" << result5 << "'" << endl;
    delete[] result5;
    
    return 0;
}
