/*
NOTE: This code requires the Crow C++ micro web framework.
To compile, you need to have the Crow header files available.
A simple way is to download 'crow_all.h' from the Crow repository
and place it in your include path.

Example compilation command with g++:
g++ -o Lse132 Lse132.cpp -I<path_to_crow_include> -lboost_system -lpthread
*/
#include "crow_all.h"
#include <string>
#include <sstream>

// This function performs HTML entity encoding to prevent Cross-Site Scripting (XSS).
// It's crucial because Crow's built-in template engine does not auto-escape.
std::string escapeHtml(const std::string& input) {
    std::ostringstream escaped;
    escaped.str("");
    for (char c : input) {
        switch (c) {
            case '<':  escaped << "&lt;"; break;
            case '>':  escaped << "&gt;"; break;
            case '&':  escaped << "&amp;"; break;
            case '"':  escaped << "&quot;"; break;
            case '\'': escaped << "&#x27;"; break;
            default:   escaped << c; break;
        }
    }
    return escaped.str();
}

// The main class name Lse132 is not applicable in this C++ style.
int main() {
    crow::SimpleApp app;

    // The content of the "hello.html" template file stored as a string.
    const std::string html_template = R"(
    <!DOCTYPE html>
    <html>
    <head>
        <title>Hello Page</title>
    </head>
    <body>
        <h1>Hello, {{ username }}!</h1>
    </body>
    </html>
    )";
    
    // The CROW_ROUTE macro binds a function (as a lambda) to a URL pattern.
    // This is the C++ equivalent of a decorator for this use case.
    // It captures parameters from the URL, like <string>.
    CROW_ROUTE(app, "/hello/<string>")
    ([&html_template](const std::string& username) {
        // We must manually escape the user-provided 'username' before rendering.
        std::string safe_username = escapeHtml(username);

        crow::mustache::template_t page(html_template);
        
        // Create a context and pass the *safe* username to the template.
        crow::mustache::context ctx;
        ctx["username"] = safe_username;
        
        return page.render(ctx);
    });

    // The main function provides test cases as URLs to visit.
    int port = 18080;
    std::string base_url = "http://127.0.0.1:" + std::to_string(port);
    std::cout << "Starting C++ Crow server on " << base_url << std::endl;
    std::cout << "--- Test Cases ---" << std::endl;
    std::cout << "Please open the following URLs in your web browser:" << std::endl;
    std::cout << "1. " << base_url << "/hello/World" << std::endl;
    std::cout << "2. " << base_url << "/hello/Test%20User" << std::endl;
    std::cout << "3. " << base_url << "/hello/O'Malley&Sons" << std::endl;
    std::cout << "4. " << base_url << "/hello/12345" << std::endl;
    std::cout << "5. " << base_url << "/hello/<script>alert('XSS')</script>" << std::endl;
    std::cout << "\nPress Ctrl+C to stop the server." << std::endl;

    app.port(port).run();
    
    return 0;
}