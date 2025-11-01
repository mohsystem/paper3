/*
 * NOTE: This code requires the 'cpp-httplib' library.
 * It is a header-only library. You must download 'httplib.h' from
 * https://github.com/yhirose/cpp-httplib and place it in the same
 * directory as this source file before compiling.
 *
 * Example compile command (with g++):
 * g++ Lse132.cpp -o Lse132_app -pthread -std=c++11
 */
#include <iostream>
#include <string>
#include "httplib.h" // The only required external header

// The main class for the application, as per instructions.
class Lse132 {
public:
    /**
     * A utility function to escape basic HTML special characters to prevent XSS.
     * @param text The string to escape.
     * @return The escaped string.
     */
    static std::string escapeHtml(const std::string& text) {
        std::string safeText;
        safeText.reserve(text.size());
        for (char c : text) {
            switch (c) {
                case '&':  safeText += "&amp;"; break;
                case '<':  safeText += "&lt;"; break;
                case '>':  safeText += "&gt;"; break;
                case '"': safeText += "&quot;"; break;
                case '\'': safeText += "&#39;"; break;
                default:   safeText += c; break;
            }
        }
        return safeText;
    }

    /**
     * This function acts as a simple template renderer.
     * It replaces a placeholder in an HTML string with the provided username.
     */
    static std::string renderHelloTemplate(const std::string& username) {
        std::string html = R"(<!doctype html>
<html>
<head><title>Greeting</title></head>
<body><h1>Hello, USERNAME!</h1></body>
</html>)";
        const std::string placeholder = "USERNAME";
        std::string safeUsername = escapeHtml(username);
        
        size_t pos = html.find(placeholder);
        if (pos != std::string::npos) {
            html.replace(pos, placeholder.length(), safeUsername);
        }
        return html;
    }

    // This function sets up and runs the web server.
    static void run() {
        httplib::Server svr;

        // Binds the GET request for "/hello/<...>" to a handler function (lambda).
        // This is the equivalent of a decorator in other languages.
        svr.Get("/hello/(.*)", [](const httplib::Request& req, httplib::Response& res) {
            // req.matches[1] contains the string captured by the (.*) regex.
            std::string username = req.matches[1];
            if (username.empty()) {
                username = "Guest";
            }
            
            std::string content = renderHelloTemplate(username);
            res.set_content(content, "text/html");
        });
        
        int port = 8081;
        std::cout << "C++ HTTP server starting on http://localhost:" << port << std::endl;
        std::cout << "To test, open your web browser and navigate to the following URLs:" << std::endl;
        std::cout << "1. http://localhost:" << port << "/hello/World" << std::endl;
        std::cout << "2. http://localhost:" << port << "/hello/Alice" << std::endl;
        std::cout << "3. http://localhost:" << port << "/hello/Bob%20Smith" << std::endl;
        std::cout << "4. http://localhost:" << port << "/hello/CPP" << std::endl;
        std::cout << "5. http://localhost:" << port << "/hello/Developer" << std::endl;
        std::cout << "Press Ctrl+C to stop the server." << std::endl;
        
        svr.listen("0.0.0.0", port);
    }
};

int main() {
    Lse132::run();
    return 0;
}