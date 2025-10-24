
#include <iostream>
#include <string>
#include <sstream>
#include <map>

// Note: This is a simplified HTTP server simulation for demonstration
// For a real web application, use a proper C++ web framework like Crow or Drogon

class Task38 {
public:
    std::string getFormPage() {
        std::stringstream html;
        html << "<!DOCTYPE html>\\n"
             << "<html>\\n"
             << "<head><title>User Input Form</title></head>\\n"
             << "<body>\\n"
             << "<h2>Enter Your Information</h2>\\n"
             << "<form method='post' action='/submit'>\\n"
             << "Name: <input type='text' name='name' required><br><br>\\n"
             << "Email: <input type='email' name='email' required><br><br>\\n"
             << "Message: <textarea name='message' rows='4' cols='50' required></textarea><br><br>\\n"
             << "<input type='submit' value='Submit'>\\n"
             << "</form>\\n"
             << "</body>\\n"
             << "</html>";
        return html.str();
    }
    
    std::string getDisplayPage(const std::string& name, const std::string& email, const std::string& message) {
        std::stringstream html;
        html << "<!DOCTYPE html>\\n"
             << "<html>\\n"
             << "<head><title>Display User Input</title></head>\\n"
             << "<body>\\n"
             << "<h2>Your Submitted Information</h2>\\n"
             << "<p><strong>Name:</strong> " << escapeHtml(name) << "</p>\\n"
             << "<p><strong>Email:</strong> " << escapeHtml(email) << "</p>\\n"
             << "<p><strong>Message:</strong> " << escapeHtml(message) << "</p>\\n"
             << "<br><a href='/'>Go Back</a>\\n"
             << "</body>\\n"
             << "</html>";
        return html.str();
    }
    
    std::string escapeHtml(const std::string& input) {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '&': output += "&amp;"; break;
                case '<': output += "&lt;"; break;
                case '>': output += "&gt;"; break;
                case '"': output += "&quot;"; break;
                case '\\'': output += "&#x27;"; break;
                default: output += c;
            }
        }
        return output;
    }
    
    std::string processInput(const std::string& name, const std::string& email, const std::string& message) {
        std::stringstream result;
        result << "Name: " << name << "\\n"
               << "Email: " << email << "\\n"
               << "Message: " << message;
        return result.str();
    }
};

int main() {
    Task38 app;
    
    std::cout << "This is a web application simulation in C++.\\n";
    std::cout << "For a real web app, use a C++ web framework.\\n\\n";
    
    std::cout << "Test cases:\\n\\n";
    
    std::string testCases[][3] = {
        {"John Doe", "john@example.com", "Hello World"},
        {"Jane Smith", "jane@test.com", "Test message"},
        {"Bob Johnson", "bob@demo.com", "This is a sample"},
        {"Alice Brown", "alice@mail.com", "C++ web app test"},
        {"Charlie Wilson", "charlie@site.com", "Final test case"}
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Test Case " << (i + 1) << ":\\n";
        std::cout << app.processInput(testCases[i][0], testCases[i][1], testCases[i][2]) << "\\n\\n";
    }
    
    return 0;
}
