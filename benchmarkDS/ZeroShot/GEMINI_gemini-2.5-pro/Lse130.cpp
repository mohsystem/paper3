#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// Note: This C++ example does not run a live web server.
// Instead, it simulates the handling of a request by processing a request URI string.
// This approach focuses on the logic requested in the prompt while remaining a single,
// dependency-free file.

/**
 * @brief Performs HTML entity escaping on a string.
 * This is a crucial security measure to prevent Cross-Site Scripting (XSS).
 *
 * @param input The string to escape.
 * @return The escaped string, safe to embed in HTML.
 */
std::string secureHtmlEscape(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&':  escaped += "&amp;";       break;
            case '<':  escaped += "&lt;";        break;
            case '>':  escaped += "&gt;";        break;
            case '"':  escaped += "&quot;";      break;
            case '\'': escaped += "&#x27;";      break;
            default:   escaped += c;             break;
        }
    }
    return escaped;
}

/**
 * @brief Renders a response based on the request "environment".
 * In this simulation, the environment is just the request URI string.
 *
 * @param request_uri The URI of the request, e.g., "/?name=Alice".
 * @return A string containing the full HTTP response.
 */
std::string render_response_from_env(const std::string& request_uri) {
    // Get the name parameter from the request URI.
    std::string name = "World";
    size_t query_pos = request_uri.find('?');
    if (query_pos != std::string::npos) {
        std::string query = request_uri.substr(query_pos + 1);
        size_t name_param_pos = query.find("name=");
        if (name_param_pos != std::string::npos) {
            std::string value = query.substr(name_param_pos + 5);
            size_t end_pos = value.find('&');
            if (end_pos != std::string::npos) {
                value = value.substr(0, end_pos);
            }
            if (!value.empty()) {
                name = value;
            }
        }
    }

    // Get the template from the "environment" (hardcoded here).
    std::string html_template = "<html><body><h1>Hello, {{name}}!</h1></body></html>";

    // Render the template with the securely escaped name parameter.
    std::string escaped_name = secureHtmlEscape(name);
    size_t placeholder_pos = html_template.find("{{name}}");
    if (placeholder_pos != std::string::npos) {
        html_template.replace(placeholder_pos, 8, escaped_name);
    }
    
    // Construct and return a full HTTP response.
    std::stringstream response_stream;
    response_stream << "HTTP/1.1 200 OK\r\n";
    response_stream << "Content-Type: text/html\r\n";
    response_stream << "Content-Length: " << html_template.length() << "\r\n";
    response_stream << "\r\n";
    response_stream << html_template;

    return response_stream.str();
}

/**
 * @brief Home function that calls the render function.
 *
 * @param request_uri The request URI.
 * @return The full HTTP response string.
 */
std::string home(const std::string& request_uri) {
    return render_response_from_env(request_uri);
}

void run_test_case(int case_num, const std::string& test_uri) {
    std::cout << "--- Test Case " << case_num << " ---\n";
    std::cout << "Input URI: " << test_uri << "\n";
    std::string response = home(test_uri);
    std::cout << "Generated Response:\n" << response << "\n\n";
}

int main() {
    // In lieu of a web server, the main method runs 5 test cases directly.
    run_test_case(1, "/?name=Alice");
    run_test_case(2, "/");
    run_test_case(3, "/?name=");
    run_test_case(4, "/?name=Bob & Charlie");
    run_test_case(5, "/?name=<script>alert('XSS')</script>");
    
    return 0;
}