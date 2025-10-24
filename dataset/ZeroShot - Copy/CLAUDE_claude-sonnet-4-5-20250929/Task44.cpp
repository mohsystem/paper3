
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <climits>

using namespace std;

const int MAX_INPUT_LENGTH = 1000;

class Task44 {
private:
    static string replaceAll(string str, const string& from, const string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return str;
    }
    
    static string trim(const string& str) {
        size_t first = str.find_first_not_of(' ');
        if (string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

public:
    static string validateAndSanitizeText(const string& input) {
        if (input.empty()) {
            return "Error: Input is empty";
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length of " + to_string(MAX_INPUT_LENGTH);
        }
        
        string trimmed = trim(input);
        
        regex alphanumericPattern("^[a-zA-Z0-9\\\\s.,!?\\\\-]+$");
        if (!regex_match(trimmed, alphanumericPattern)) {
            return "Error: Input contains invalid characters";
        }
        
        string sanitized = replaceAll(trimmed, "&", "&amp;");
        sanitized = replaceAll(sanitized, "<", "&lt;");
        sanitized = replaceAll(sanitized, ">", "&gt;");
        sanitized = replaceAll(sanitized, "\\"", "&quot;");
        sanitized = replaceAll(sanitized, "'", "&#x27;");
        
        return "Valid text: " + sanitized;
    }
    
    static string validateEmail(const string& email) {
        if (email.empty()) {
            return "Error: Email is null or empty";
        }
        
        if (email.length() > 254) {
            return "Error: Email exceeds maximum length";
        }
        
        regex emailPattern("^[a-zA-Z0-9._%+\\\\-]+@[a-zA-Z0-9.\\\\-]+\\\\.[a-zA-Z]{2,}$");
        if (!regex_match(email, emailPattern)) {
            return "Error: Invalid email format";
        }
        
        return "Valid email: " + email;
    }
    
    static string validateAndProcessNumber(const string& input) {
        if (input.empty()) {
            return "Error: Number input is null or empty";
        }
        
        regex numericPattern("^-?\\\\d+$");
        if (!regex_match(input, numericPattern)) {
            return "Error: Invalid number format";
        }
        
        try {
            long long number = stoll(input);
            long long squared = number * number;
            return "Number: " + to_string(number) + ", Squared: " + to_string(squared);
        } catch (...) {
            return "Error: Number out of range";
        }
    }
    
    static string processOperation(const string& operationType, const string& input) {
        if (operationType.empty() || input.empty()) {
            return "Error: Operation type or input is null";
        }
        
        string opType = operationType;
        transform(opType.begin(), opType.end(), opType.begin(), ::tolower);
        
        if (opType == "text") {
            return validateAndSanitizeText(input);
        } else if (opType == "email") {
            return validateEmail(input);
        } else if (opType == "number") {
            return validateAndProcessNumber(input);
        } else {
            return "Error: Unknown operation type";
        }
    }
};

int main() {
    cout << "=== Test Case 1: Valid Text ===" << endl;
    string result1 = Task44::processOperation("text", "Hello World 123!");
    cout << result1 << endl;
    
    cout << "\\n=== Test Case 2: Invalid Text with Special Characters ===" << endl;
    string result2 = Task44::processOperation("text", "<script>alert('XSS')</script>");
    cout << result2 << endl;
    
    cout << "\\n=== Test Case 3: Valid Email ===" << endl;
    string result3 = Task44::processOperation("email", "user@example.com");
    cout << result3 << endl;
    
    cout << "\\n=== Test Case 4: Valid Number ===" << endl;
    string result4 = Task44::processOperation("number", "42");
    cout << result4 << endl;
    
    cout << "\\n=== Test Case 5: Invalid Number ===" << endl;
    string result5 = Task44::processOperation("number", "abc123");
    cout << result5 << endl;
    
    return 0;
}
