
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>

// HTML entity encoding to prevent XSS attacks
// This sanitizes user input by converting special characters to HTML entities
std::string htmlEncode(const std::string& input) {
    // Validate input length to prevent excessive memory usage
    const size_t MAX_INPUT_LENGTH = 100000;
    if (input.length() > MAX_INPUT_LENGTH) {
        return "[Input too large]";
    }
    
    std::ostringstream encoded;
    // Process each character and encode dangerous HTML characters
    for (size_t i = 0; i < input.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        switch (c) {
            case '&':  encoded << "&amp;"; break;   // Encode ampersand
            case '<':  encoded << "&lt;"; break;    // Encode less-than (prevents tag injection)
            case '>':  encoded << "&gt;"; break;    // Encode greater-than (prevents tag injection)
            case '"':  encoded << "&quot;"; break;  // Encode quotes (prevents attribute injection)
            case '\\'': encoded << "&#x27;"; break;  // Encode single quote (prevents attribute injection)
            case '/':  encoded << "&#x2F;"; break;  // Encode forward slash (prevents closing tags)
            default:
                // Only allow printable ASCII and common whitespace
                if ((c >= 32 && c <= 126) || c == '\\n' || c == '\\r' || c == '\\t') {
                    encoded << c;
                } else {
                    // Replace non-printable characters with safe placeholder
                    encoded << "&#xFFFD;";
                }
                break;
        }
    }
    return encoded.str();
}

// Generate complete HTML page with encoded user input
std::string generateWebPage(const std::string& userInput) {
    // Validate input is not null or excessively large
    if (userInput.empty()) {
        return "<!DOCTYPE html>\\n<html lang=\\"en\\">\\n<head>\\n"
               "<meta charset=\\"UTF-8\\">\\n"
               "<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'; script-src 'none'; object-src 'none';\\">\\n"
               "<title>User Input Display</title>\\n</head>\\n"
               "<body>\\n<h1>User Input:</h1>\\n<p>[No input provided]</p>\\n</body>\\n</html>";
    }
    
    // Sanitize input using HTML entity encoding
    std::string safeInput = htmlEncode(userInput);
    
    // Build HTML with Content Security Policy header to prevent XSS
    std::ostringstream html;
    html << "<!DOCTYPE html>\\n"
         << "<html lang=\\"en\\">\\n"
         << "<head>\\n"
         << "<meta charset=\\"UTF-8\\">\\n"
         << "<meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n"
         // CSP prevents inline scripts and external resources from executing
         << "<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'; script-src 'none'; object-src 'none';\\">\\n"
         << "<title>User Input Display</title>\\n"
         << "</head>\\n"
         << "<body>\\n"
         << "<h1>User Input:</h1>\\n"
         << "<div style=\\"border:1px solid #ccc; padding:10px; margin:10px;\\">\\n"
         << "<pre>" << safeInput << "</pre>\\n"  // Using pre tag to preserve formatting
         << "</div>\\n"
         << "</body>\\n"
         << "</html>";
    
    return html.str();
}

int main() {
    // Test case 1: Normal text input
    {
        std::string input = "Hello, World!";
        std::string page = generateWebPage(input);
        std::cout << "Test 1 - Normal text:\\n" << page << "\\n\\n";
    }
    
    // Test case 2: XSS attempt with script tags
    {
        std::string input = "<script>alert('XSS')</script>";
        std::string page = generateWebPage(input);
        std::cout << "Test 2 - Script injection attempt:\\n" << page << "\\n\\n";
    }
    
    // Test case 3: HTML injection with image tag
    {
        std::string input = "<img src=x onerror=alert('XSS')>";
        std::string page = generateWebPage(input);
        std::cout << "Test 3 - Image tag injection:\\n" << page << "\\n\\n";
    }
    
    // Test case 4: Special characters and quotes
    {
        std::string input = "Test \\"quotes\\" & 'apostrophes' <tags>";
        std::string page = generateWebPage(input);
        std::cout << "Test 4 - Special characters:\\n" << page << "\\n\\n";
    }
    
    // Test case 5: Empty input
    {
        std::string input = "";
        std::string page = generateWebPage(input);
        std::cout << "Test 5 - Empty input:\\n" << page << "\\n\\n";
    }
    
    return 0;
}
