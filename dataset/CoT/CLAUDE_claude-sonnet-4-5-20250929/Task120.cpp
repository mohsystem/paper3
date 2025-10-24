
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

class Task120 {
public:
    // Method to safely encode user input for HTML display
    static std::string encodeForHTML(const std::string& input) {
        std::string encoded;
        encoded.reserve(input.length() * 1.5); // Reserve space to avoid reallocations
        
        for (char c : input) {
            switch (c) {
                case '&':
                    encoded.append("&amp;");
                    break;
                case '<':
                    encoded.append("&lt;");
                    break;
                case '>':
                    encoded.append("&gt;");
                    break;
                case '"':
                    encoded.append("&quot;");
                    break;
                case '\\'':
                    encoded.append("&#x27;");
                    break;
                case '/':
                    encoded.append("&#x2F;");
                    break;
                default:
                    encoded.push_back(c);
                    break;
            }
        }
        return encoded;
    }
    
    // Method to display user input safely
    static std::string displayUserInput(const std::string& userInput) {
        std::string safeInput = encodeForHTML(userInput);
        std::ostringstream html;
        html << "<html>\\n"
             << "<head>\\n"
             << "    <meta charset=\\"UTF-8\\">\\n"
             << "    <title>User Input Display</title>\\n"
             << "</head>\\n"
             << "<body>\\n"
             << "    <h1>User Input Display</h1>\\n"
             << "    <p>You entered: " << safeInput << "</p>\\n"
             << "</body>\\n"
             << "</html>";
        return html.str();
    }
};

int main() {
    std::cout << "Test Case 1 - Normal input:" << std::endl;
    std::cout << Task120::displayUserInput("Hello World") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2 - Input with HTML tags:" << std::endl;
    std::cout << Task120::displayUserInput("<script>alert('XSS')</script>") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3 - Input with special characters:" << std::endl;
    std::cout << Task120::displayUserInput("Tom & Jerry's \\"Adventure\\"") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4 - Input with single quotes:" << std::endl;
    std::cout << Task120::displayUserInput("It's a beautiful day") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5 - Input with mixed content:" << std::endl;
    std::cout << Task120::displayUserInput("<div>Hello</div> & <span>World</span>") << std::endl;
    
    return 0;
}
