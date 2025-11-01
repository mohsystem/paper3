#define CROW_MAIN
#include "crow.h"
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

// A simple HTML escaper to prevent XSS.
// In a real application, use a robust, well-tested library.
std::string escapeHtml(const std::string& input) {
    std::ostringstream escaped;
    escaped.str().reserve(input.length());
    for (char c : input) {
        switch (c) {
            case '&':  escaped << "&amp;";       break;
            case '<':  escaped << "&lt;";        break;
            case '>':  escaped << "&gt;";        break;
            case '"':  escaped << "&quot;";      break;
            case '\'': escaped << "&#39;";       break;
            default:   escaped << c;             break;
        }
    }
    return escaped.str();
}

// Function to replace a placeholder in a string
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

// This function is bound to the route /hello/<username>
crow::response hello(const std::string& username) {
    // Rule #1: Validate input
    // Username should be alphanumeric + underscore, 1 to 50 chars.
    const std::regex username_pattern("^[a-zA-Z0-9_]{1,50}$");
    if (!std::regex_match(username, username_pattern)) {
        return crow::response(400, "Invalid username format.");
    }

    // Safely read the template file. Rules #5 and #6.
    // The path is hardcoded, preventing path traversal.
    std::ifstream template_file("hello.html", std::ios::in | std::ios::binary);
    if (!template_file.is_open()) {
        return crow::response(500, "Internal Server Error: Template not found.");
    }
    
    std::stringstream buffer;
    buffer << template_file.rdbuf();
    std::string content = buffer.str();
    template_file.close();

    // Rule #2: Sanitize output to prevent XSS.
    std::string sanitized_username = escapeHtml(username);

    // Render the template
    replaceAll(content, "{{username}}", sanitized_username);

    return crow::response(content);
}

void create_template_file() {
    std::ofstream ofs("hello.html");
    if (ofs.is_open()) {
        ofs << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<head><title>Hello</title></head>\n"
            << "<body>\n"
            << "    <h1>Hello, {{username}}!</h1>\n"
            << "</body>\n"
            << "</html>\n";
        ofs.close();
        std::cout << "Created 'hello.html' template file." << std::endl;
    }
}

int main() {
    create_template_file();
    
    crow::SimpleApp app;

    // A decorator-like macro is used to bind a function to a URL.
    CROW_ROUTE(app, "/hello/<string>")(hello);

    std::cout << "Server starting on http://localhost:18080" << std::endl;
    std::cout << "Try the following 5 test cases in your browser or with curl:" << std::endl;
    std::cout << "1. Valid: http://localhost:18080/hello/Alice" << std::endl;
    std::cout << "2. Valid with numbers: http://localhost:18080/hello/user123" << std::endl;
    std::cout << "3. Valid with underscore: http://localhost:18080/hello/test_user" << std::endl;
    std::cout << "4. Invalid characters: http://localhost:18080/hello/bad!user (should return 400 Bad Request)" << std::endl;
    std::cout << "5. XSS attempt: http://localhost:18080/hello/%3Cscript%3Ealert(1)%3C/script%3E (should display escaped text)" << std::endl;

    app.port(18080).run();

    return 0;
}