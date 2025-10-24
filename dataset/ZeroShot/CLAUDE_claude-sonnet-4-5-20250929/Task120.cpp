
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

class Task120 {
public:
    // HTML escape function to prevent XSS attacks
    static string escapeHtml(const string& input) {
        if (input.empty()) {
            return "";
        }
        ostringstream escaped;
        for (char c : input) {
            switch (c) {
                case '<':
                    escaped << "&lt;";
                    break;
                case '>':
                    escaped << "&gt;";
                    break;
                case '&':
                    escaped << "&amp;";
                    break;
                case '"':
                    escaped << "&quot;";
                    break;
                case '\\'':
                    escaped << "&#x27;";
                    break;
                case '/':
                    escaped << "&#x2F;";
                    break;
                default:
                    escaped << c;
            }
        }
        return escaped.str();
    }
    
    static string displayUserInput(const string& userInput) {
        string sanitizedInput = escapeHtml(userInput);
        ostringstream html;
        html << "<!DOCTYPE html>\\n";
        html << "<html>\\n";
        html << "<head>\\n";
        html << "    <meta charset=\\"UTF-8\\">\\n";
        html << "    <title>User Input Display</title>\\n";
        html << "</head>\\n";
        html << "<body>\\n";
        html << "    <h1>User Input Display</h1>\\n";
        html << "    <p>You entered: " << sanitizedInput << "</p>\\n";
        html << "</body>\\n";
        html << "</html>";
        return html.str();
    }
};

int main() {
    cout << "=== Test Case 1: Normal Text ===" << endl;
    string test1 = "Hello, World!";
    cout << Task120::displayUserInput(test1) << endl << endl;
    
    cout << "=== Test Case 2: XSS Attack Attempt (Script Tag) ===" << endl;
    string test2 = "<script>alert('XSS')</script>";
    cout << Task120::displayUserInput(test2) << endl << endl;
    
    cout << "=== Test Case 3: Special Characters ===" << endl;
    string test3 = "Price: $100 & Free Shipping!";
    cout << Task120::displayUserInput(test3) << endl << endl;
    
    cout << "=== Test Case 4: HTML Injection Attempt ===" << endl;
    string test4 = "<img src=x onerror=alert('XSS')>";
    cout << Task120::displayUserInput(test4) << endl << endl;
    
    cout << "=== Test Case 5: Mixed Content ===" << endl;
    string test5 = "My email: user@example.com & phone: 555-1234";
    cout << Task120::displayUserInput(test5) << endl;
    
    return 0;
}
