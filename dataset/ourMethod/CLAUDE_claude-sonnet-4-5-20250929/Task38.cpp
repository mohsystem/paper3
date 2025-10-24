
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cctype>

// HTML encode function to prevent XSS attacks
// Escapes <, >, &, ", ' characters to their HTML entity equivalents\nstd::string htmlEncode(const std::string& input) {\n    std::string output;\n    output.reserve(input.length() * 2); // Pre-allocate to reduce reallocations\n    \n    for (char c : input) {\n        switch (c) {\n            case '<':\n                output.append("&lt;");\n                break;\n            case '>':\n                output.append("&gt;");\n                break;\n            case '&':\n                output.append("&amp;");\n                break;\n            case '"':\n                output.append("&quot;");\n                break;\n            case '\\'':
                output.append("&#x27;");
                break;
            default:
                output.push_back(c);
                break;
        }
    }
    return output;
}

// Validate input: check length and reject control characters except newlines/tabs
// Returns true if input is valid, false otherwise
bool validateInput(const std::string& input, size_t maxLength) {
    // Rule#3: Validate input length to prevent excessive memory usage
    if (input.length() > maxLength) {
        return false;
    }
    
    // Rule#3: Check for dangerous control characters
    for (unsigned char c : input) {
        // Allow printable characters, newlines, tabs, and carriage returns
        if (c < 32 && c != '\\n' && c != '\\r' && c != '\\t') {
            return false;
        }
    }
    
    return true;
}

// Generate HTML page with form and optional display of user input
// Takes user input and generates complete HTML response
std::string generateWebPage(const std::string& userInput) {
    const size_t MAX_INPUT_LENGTH = 1000; // Rule#3: Define maximum input length
    
    std::ostringstream html;
    
    // Rule#1: Use constant format strings for HTML generation
    html << "<!DOCTYPE html>\\n"
         << "<html lang=\\"en\\">\\n"
         << "<head>\\n"
         << "    <meta charset=\\"UTF-8\\">\\n"
         << "    <meta http-equiv=\\"X-UA-Compatible\\" content=\\"IE=edge\\">\\n"
         << "    <meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n"
         << "    <meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'; script-src 'none'; object-src 'none';\\">\\n"
         << "    <title>User Input Form</title>\\n"
         << "    <style>\\n"
         << "        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; }\\n"
         << "        .container { border: 1px solid #ccc; padding: 20px; border-radius: 5px; }\\n"
         << "        .form-group { margin-bottom: 15px; }\\n"
         << "        label { display: block; margin-bottom: 5px; font-weight: bold; }\\n"
         << "        input[type=\\"text\\"] { width: 100%; padding: 8px; box-sizing: border-box; }\\n"
         << "        button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; cursor: pointer; }\\n"
         << "        button:hover { background-color: #45a049; }\\n"
         << "        .output { margin-top: 20px; padding: 15px; background-color: #f0f0f0; border-radius: 5px; }\\n"
         << "    </style>\\n"
         << "</head>\\n"
         << "<body>\\n"
         << "    <div class=\\"container\\">\\n"
         << "        <h1>User Input Form</h1>\\n"
         << "        <form method=\\"GET\\" action=\\"\\">\\n"
         << "            <div class=\\"form-group\\">\\n"
         << "                <label for=\\"userInput\\">Enter your message (max " << MAX_INPUT_LENGTH << " characters):</label>\\n"
         << "                <input type=\\"text\\" id=\\"userInput\\" name=\\"userInput\\" maxlength=\\"" << MAX_INPUT_LENGTH << "\\" required>\\n"
         << "            </div>\\n"
         << "            <button type=\\"submit\\">Submit</button>\\n"
         << "        </form>\\n";
    
    // Rule#3: Validate and sanitize user input before displaying
    if (!userInput.empty()) {
        if (validateInput(userInput, MAX_INPUT_LENGTH)) {
            // Rule#2: HTML encode output to prevent XSS attacks
            std::string safeInput = htmlEncode(userInput);
            
            html << "        <div class=\\"output\\">\\n"
                 << "            <h2>Your Input:</h2>\\n"
                 << "            <p>" << safeInput << "</p>\\n"
                 << "        </div>\\n";
        } else {
            // Rule#3: Reject invalid input with clear error message
            html << "        <div class=\\"output\\" style=\\"background-color: #ffcccc;\\">\\n"
                 << "            <h2>Error:</h2>\\n"
                 << "            <p>Invalid input. Please ensure your input is within the allowed length and contains no control characters.</p>\\n"
                 << "        </div>\\n";
        }
    }
    
    html << "    </div>\\n"
         << "</body>\\n"
         << "</html>\\n";
    
    return html.str();
}

// Simulate parsing query string from GET request
// Extracts userInput parameter value from query string
std::string parseQueryString(const std::string& queryString) {
    const std::string prefix = "userInput=";
    size_t pos = queryString.find(prefix);
    
    if (pos == std::string::npos) {
        return "";
    }
    
    size_t start = pos + prefix.length();
    size_t end = queryString.find('&', start);
    
    std::string value;
    if (end == std::string::npos) {
        value = queryString.substr(start);
    } else {
        value = queryString.substr(start, end - start);
    }
    
    // Rule#3: Basic URL decoding for spaces (+ to space)
    std::replace(value.begin(), value.end(), '+', ' ');
    
    return value;
}

int main() {
    // Test case 1: Empty input (displays form only)
    std::cout << "=== Test Case 1: Empty Input ===\\n";
    std::string result1 = generateWebPage("");
    std::cout << "Generated " << result1.length() << " bytes of HTML\\n";
    std::cout << (result1.find("<form method=\\"GET\\"") != std::string::npos ? "PASS" : "FAIL") << "\\n\\n";
    
    // Test case 2: Normal text input
    std::cout << "=== Test Case 2: Normal Text ===\\n";
    std::string input2 = "Hello, World!";
    std::string result2 = generateWebPage(input2);
    std::cout << "Input: " << input2 << "\\n";
    std::cout << (result2.find("Hello, World!") != std::string::npos ? "PASS" : "FAIL") << "\\n\\n";
    
    // Test case 3: Input with HTML special characters (XSS attempt)
    std::cout << "=== Test Case 3: XSS Prevention ===\\n";
    std::string input3 = "<script>alert('XSS')</script>";
    std::string result3 = generateWebPage(input3);
    std::cout << "Input: " << input3 << "\\n";
    std::cout << (result3.find("&lt;script&gt;") != std::string::npos ? "PASS (XSS blocked)" : "FAIL") << "\\n";
    std::cout << (result3.find("<script>") == std::string::npos ? "PASS (no raw script)" : "FAIL") << "\\n\\n";
    
    // Test case 4: Input with quotes and ampersands
    std::cout << "=== Test Case 4: Special Characters ===\\n";
    std::string input4 = "Tom's \\"great\\" idea & more";
    std::string result4 = generateWebPage(input4);
    std::cout << "Input: " << input4 << "\\n";
    std::cout << (result4.find("&quot;") != std::string::npos ? "PASS (quotes encoded)" : "FAIL") << "\\n";
    std::cout << (result4.find("&amp;") != std::string::npos ? "PASS (ampersand encoded)" : "FAIL") << "\\n\\n";
    
    // Test case 5: Input exceeding maximum length
    std::cout << "=== Test Case 5: Length Validation ===\\n";
    std::string input5(1500, 'A'); // 1500 characters
    std::string result5 = generateWebPage(input5);
    std::cout << "Input length: " << input5.length() << " characters\\n";
    std::cout << (result5.find("Invalid input") != std::string::npos ? "PASS (rejected)" : "FAIL") << "\\n";
    std::cout << (result5.find(std::string(1500, 'A')) == std::string::npos ? "PASS (not displayed)" : "FAIL") << "\\n\\n";
    
    return 0;
}
