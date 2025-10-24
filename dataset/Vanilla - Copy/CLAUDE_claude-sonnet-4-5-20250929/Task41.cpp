
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class Task41 {
public:
    static string processString(const string& input) {
        // Using std::string which automatically manages memory
        // and prevents buffer overflow
        return "Processed: " + input;
    }
    
    // Alternative safe C-style implementation with bounds checking
    static void processStringCStyle(const char* input, char* output, size_t outputSize) {
        if (input == nullptr || output == nullptr || outputSize == 0) {
            return;
        }
        
        const char* prefix = "Processed: ";
        size_t prefixLen = strlen(prefix);
        size_t inputLen = strlen(input);
        
        // Ensure we don't overflow the output buffer\n        if (prefixLen + inputLen + 1 > outputSize) {\n            // Truncate input to fit in buffer\n            size_t availableSpace = outputSize - prefixLen - 1;\n            strncpy(output, prefix, outputSize - 1);\n            strncat(output, input, availableSpace);\n        } else {\n            strcpy(output, prefix);\n            strcat(output, input);\n        }\n        \n        output[outputSize - 1] = '\\0'; // Ensure null termination\n    }\n};\n\nint main() {\n    // Test case 1: Normal string\n    cout << "Test 1: " << Task41::processString("Hello World") << endl;\n    \n    // Test case 2: Empty string\n    cout << "Test 2: " << Task41::processString("") << endl;\n    \n    // Test case 3: Very long string\n    string longString(10000, 'a');\n    string result = Task41::processString(longString);\n    cout << "Test 3: " << result.substr(0, 50) << "..." << endl;\n    \n    // Test case 4: Special characters\n    cout << "Test 4: " << Task41::processString("!@#$%^&*()_+-=[]{}|;':,.<>?") << endl;\n    \n    // Test case 5: C-style safe processing\n    char buffer[100];\n    Task41::processStringCStyle("Safe buffer handling", buffer, sizeof(buffer));\n    cout << "Test 5: " << buffer << endl;
    
    return 0;
}
