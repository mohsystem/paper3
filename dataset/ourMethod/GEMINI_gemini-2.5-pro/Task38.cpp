#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>

// Function to escape HTML special characters to prevent XSS.
// All user-controlled input that is reflected back to a web page
// must be properly escaped.
std::string html_escape(const std::string& data) {
    std::string escaped;
    // Pre-allocating memory can be a performance optimization.
    // The new string will be at least as long as the original.
    escaped.reserve(data.length()); 
    for (char c : data) {
        switch (c) {
            case '&':  escaped.append("&amp;");       break;
            case '\"': escaped.append("&quot;");      break;
            case '\'': escaped.append("&#39;");      break;
            case '<':  escaped.append("&lt;");        break;
            case '>':  escaped.append("&gt;");        break;
            default:   escaped.push_back(c);        break;
        }
    }
    return escaped;
}

// Function to parse URL-encoded form data (e.g., "key=value").
// It finds the 'user_input' key, extracts its value, and URL-decodes it.
std::string parse_form_data(const std::string& data) {
    std::string key = "user_input=";
    size_t pos = data.find(key);
    if (pos == std::string::npos) {
        return "";
    }
    std::string value_encoded = data.substr(pos + key.length());
    
    // URL Decode the value
    std::string value_decoded;
    value_decoded.reserve(value_encoded.length());
    for (size_t i = 0; i < value_encoded.length(); ++i) {
        if (value_encoded[i] == '+') {
            value_decoded.push_back(' ');
        } else if (value_encoded[i] == '%' && i + 2 < value_encoded.length()) {
            try {
                std::string hex = value_encoded.substr(i + 1, 2);
                // stol with base 16 converts hex string to integer
                char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
                value_decoded.push_back(decoded_char);
                i += 2;
            } catch (const std::invalid_argument& e) {
                // If hex is invalid, append the '%' literally as a fallback
                value_decoded.push_back('%');
            } catch (const std::out_of_range& e) {
                 // If hex value is out of range, append the '%' literally
                value_decoded.push_back('%');
            }
        } else {
            value_decoded.push_back(value_encoded[i]);
        }
    }
    return value_decoded;
}

// Prints the HTML form to standard output.
void display_form() {
    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << "<!DOCTYPE html>\n"
              << "<html lang=\"en\">\n"
              << "<head><meta charset=\"UTF-8\"><title>C++ CGI Form</title></head>\n"
              << "<body>\n"
              << "<h1>Enter some text</h1>\n"
              // An empty action attribute makes the form submit to the same URL.
              << "<form method=\"POST\" action=\"\">\n"
              << "  <input type=\"text\" name=\"user_input\" size=\"50\">\n"
              << "  <input type=\"submit\" value=\"Submit\">\n"
              << "</form>\n"
              << "</body>\n"
              << "</html>\n";
}

// Parses the input, escapes it, and prints the response page.
void display_response(const std::string& input) {
    std::string decoded_input = parse_form_data(input);
    std::string safe_input = html_escape(decoded_input);

    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << "<!DOCTYPE html>\n"
              << "<html lang=\"en\">\n"
              << "<head><meta charset=\"UTF-8\"><title>C++ CGI Response</title></head>\n"
              << "<body>\n"
              << "<h1>You entered:</h1>\n"
              // Displaying the escaped input prevents XSS attacks.
              << "<p>" << safe_input << "</p>\n"
              << "<a href=\"\">Go back</a>\n"
              << "</body>\n"
              << "</html>\n";
}

// Function to run test cases when not in a CGI environment.
void run_tests() {
    std::cout << "Running tests...\n";
    
    const std::vector<std::pair<std::string, std::string>> escape_tests = {
        {"hello", "hello"},
        {"<script>alert(1)</script>", "&lt;script&gt;alert(1)&lt;/script&gt;"},
        {"' or 1=1 --", "&#39; or 1=1 --"},
        {"& \" < >", "&amp; &quot; &lt; &gt;"},
        {"", ""}
    };
    std::cout << "\n--- html_escape tests ---\n";
    for(size_t i = 0; i < escape_tests.size(); ++i) {
        const auto& test = escape_tests[i];
        std::string result = html_escape(test.first);
        std::cout << "Test " << i + 1 << ": " << (result == test.second ? "PASSED" : "FAILED") << std::endl;
    }

    const std::vector<std::pair<std::string, std::string>> parse_tests = {
        {"user_input=hello+world", "hello world"},
        {"user_input=some%20text%26more", "some text&more"},
        {"another_key=value&user_input=test", "test"},
        {"user_input=", ""},
        {"user_input=test%XX", "test%XX"} // Invalid hex
    };
    std::cout << "\n--- parse_form_data tests ---\n";
    for(size_t i = 0; i < parse_tests.size(); ++i) {
        const auto& test = parse_tests[i];
        std::string result = parse_form_data(test.first);
        std::cout << "Test " << i + 1 << ": " << (result == test.second ? "PASSED" : "FAILED") << std::endl;
    }
}

int main() {
    // A CGI program determines its context via environment variables.
    // If REQUEST_METHOD is set, we assume it's running as a CGI script.
    const char* request_method_env = std::getenv("REQUEST_METHOD");
    if (request_method_env) {
        std::string request_method(request_method_env);

        if (request_method == "POST") {
            const char* content_length_env = std::getenv("CONTENT_LENGTH");
            if (content_length_env) {
                try {
                    long long content_length = std::stoll(content_length_env);
                    // Set a reasonable limit to prevent resource exhaustion.
                    const long long MAX_LENGTH = 1024 * 10; // 10KB
                    if (content_length > 0 && content_length <= MAX_LENGTH) {
                        std::vector<char> buffer(content_length);
                        // Read the POST data from standard input.
                        std::cin.read(buffer.data(), content_length);
                        std::string post_data(buffer.data(), content_length);
                        display_response(post_data);
                    } else {
                        // If content length is invalid or too large, show the form.
                        display_form();
                    }
                } catch (const std::exception& e) {
                     // If CONTENT_LENGTH is not a valid number, show the form.
                     display_form();
                }
            } else {
                 display_form();
            }
        } else { // Default to GET for any other method
            display_form();
        }
    } else {
        // If not in a CGI environment, run tests to verify core logic.
        run_tests();
    }

    return 0;
}