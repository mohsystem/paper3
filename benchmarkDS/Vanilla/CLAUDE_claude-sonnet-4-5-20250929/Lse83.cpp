
#include <iostream>
#include <string>
#include <regex>
#include <sstream>

using namespace std;

class Lse83 {
public:
    static string processPayload(const string& payload) {
        try {
            // Simple JSON parser for "name" field
            regex namePattern("\\"name\\"\\\\s*:\\\\s*\\"([^\\"]*)\\"");
            smatch match;
            
            if (regex_search(payload, match, namePattern)) {
                if (match.size() > 1) {
                    return match[1].str();
                }
            }
            
            // Check if payload looks like valid JSON but doesn't have name\n            if (payload.find("{") != string::npos && payload.find("}") != string::npos) {\n                return "Field 'name' not found";\n            }\n            \n            return "Invalid JSON payload";\n        } catch (...) {\n            return "Invalid JSON payload";\n        }\n    }\n};\n\nint main() {\n    // Test cases\n    cout << "Test Case 1: " << Lse83::processPayload("{\\"name\\":\\"John Doe\\"}") << endl;
    cout << "Test Case 2: " << Lse83::processPayload("{\\"name\\":\\"Alice\\",\\"age\\":30}") << endl;
    cout << "Test Case 3: " << Lse83::processPayload("{\\"age\\":25}") << endl;
    cout << "Test Case 4: " << Lse83::processPayload("{\\"name\\":\\"\\"}") << endl;
    cout << "Test Case 5: " << Lse83::processPayload("invalid json") << endl;
    
    return 0;
}
