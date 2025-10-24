
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

class Task120 {
public:
    // Method to sanitize HTML to prevent XSS attacks
    static string sanitizeHTML(const string& input) {
        if (input.empty()) {
            return "";
        }
        
        string result;
        for (char c : input) {
            switch (c) {
                case '&':
                    result += "&amp;";
                    break;
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '"':
                    result += "&quot;";
                    break;
                case '\\'':
                    result += "&#x27;";
                    break;
                case '/':
                    result += "&#x2F;";
                    break;
                default:
                    result += c;
            }
        }
        return result;
    }
    
    // Method to generate HTML page with user input
    static string generateWebPage(const string& userInput) {
        string sanitizedInput = sanitizeHTML(userInput);
        
        ostringstream html;
        html << "<!DOCTYPE html>\\n";
        html << "<html>\\n";
        html << "<head>\\n";
        html << "    <title>User Input Display</title>\\n";
        html << "    <style>\\n";
        html << "        body { font-family: Arial, sans-serif; margin: 40px; }\\n";
        html << "        .container { border: 2px solid #333; padding: 20px; border-radius: 5px; }\\n";
        html << "        h1 { color: #333; }\\n";
        html << "    </style>\\n";
        html << "</head>\\n";
        html << "<body>\\n";
        html << "    <div class=\\"container\\">\\n";
        html << "        <h1>User Input Display</h1>\\n";
        html << "        <p><strong>You entered:</strong></p>\\n";
        html << "        <p>" << sanitizedInput << "</p>\\n";
        html << "    </div>\\n";
        html << "</body>\\n";
        html << "</html>";
        
        return html.str();
    }
};

int main() {
    cout << "Test Case 1: Simple text" << endl;
    cout << Task120::generateWebPage("Hello, World!") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 2: Text with HTML tags (XSS prevention)" << endl;
    cout << Task120::generateWebPage("<script>alert('XSS')</script>") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 3: Text with special characters" << endl;
    cout << Task120::generateWebPage("Price: $100 & up, 50% off!") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 4: Multi-line text" << endl;
    cout << Task120::generateWebPage("Line 1\\nLine 2\\nLine 3") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 5: Empty input" << endl;
    cout << Task120::generateWebPage("") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    return 0;
}
