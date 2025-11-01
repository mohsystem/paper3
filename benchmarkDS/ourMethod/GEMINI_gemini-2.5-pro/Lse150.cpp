// This code requires the httplib.h header file.
// You can get it from: https://github.com/yhirose/cpp-httplib
// Place httplib.h in the same directory or in your include path.
// To compile: g++ -std=c++11 your_file.cpp -o your_app -pthread
#include <iostream>
#include <string>
#include <regex>
#include "httplib.h"

// --- Constants ---
const size_t MAX_USERNAME_LENGTH = 32;
const size_t MAX_MESSAGE_LENGTH = 256;
// Simple alphanumeric pattern for username validation
const std::regex USERNAME_PATTERN("^[a-zA-Z0-9_]+$");

// --- Mock Database Function ---
bool insert_user_message_in_db(const std::string& username, const std::string& message) {
    // In a real application, use prepared statements to prevent SQL injection.
    std::cout << "DATABASE: Inserting message for user '" << username << "': '" << message << "'" << std::endl;
    return true; // Simulate success
}

// --- Core Logic for Testing ---
std::string handle_post_logic(const std::string& username, const std::string& message) {
    if (username.empty()) {
        return "Username cannot be empty.";
    }
    if (message.empty()) {
        return "Message cannot be empty.";
    }
    if (username.length() > MAX_USERNAME_LENGTH) {
        return "Username is too long.";
    }
    if (message.length() > MAX_MESSAGE_LENGTH) {
        return "Message is too long.";
    }
    if (!std::regex_match(username, USERNAME_PATTERN)) {
        return "Username contains invalid characters.";
    }

    if (insert_user_message_in_db(username, message)) {
        return "Success";
    } else {
        return "Failed to save message.";
    }
}

// --- `post()` function to be called on submission ---
void post(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    std::string message;

    if (req.has_param("username")) {
        username = req.get_param_value("username");
    }
    if (req.has_param("message")) {
        message = req.get_param_value("message");
    }

    std::string result = handle_post_logic(username, message);
    
    if (result == "Success") {
        // Redirect the user back to the main page
        res.set_redirect("/");
    } else {
        // Handle error
        std::string error_html = "<html><body><h1>Error: " + result + "</h1><a href=\"/\">Go back</a></body></html>";
        res.set_content(error_html, "text/html");
        res.status = 400; // Bad Request
    }
}

// --- Test Cases ---
void run_tests() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    // Test Case 1: Valid input
    std::cout << "Test 1: " << handle_post_logic("testuser", "Hello World!") << std::endl;
    // Test Case 2: Empty username
    std::cout << "Test 2: " << handle_post_logic("", "A message from nowhere.") << std::endl;
    // Test Case 3: Empty message
    std::cout << "Test 3: " << handle_post_logic("anotheruser", "") << std::endl;
    // Test Case 4: Username too long
    std::string long_username(MAX_USERNAME_LENGTH + 1, 'a');
    std::cout << "Test 4: " << handle_post_logic(long_username, "Short message.") << std::endl;
    // Test Case 5: Message too long
    std::string long_message(MAX_MESSAGE_LENGTH + 1, 'b');
    std::cout << "Test 5: " << handle_post_logic("shortuser", long_message) << std::endl;
    std::cout << "--- Test Cases Finished ---\n" << std::endl;
}

int main() {
    run_tests();

    httplib::Server svr;

    // Serve the main page with the form
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string form_html = R"(
            <!DOCTYPE html>
            <html>
            <head><title>Message Board</title></head>
            <body>
                <h1>Post a Message</h1>
                <form action="/post" method="post">
                    <label for="username">Username:</label><br>
                    <input type="text" id="username" name="username" required maxlength="32"><br><br>
                    <label for="message">Message:</label><br>
                    <textarea id="message" name="message" rows="4" cols="50" required maxlength="256"></textarea><br><br>
                    <input type="submit" value="Submit">
                </form>
            </body>
            </html>
        )";
        res.set_content(form_html, "text/html");
    });

    // Handle the form submission
    svr.Post("/post", post);

    int port = 8080;
    std::cout << "Server starting on port " << port << std::endl;
    std::cout << "Open http://localhost:" << port << " in your browser." << std::endl;
    if (!svr.listen("localhost", port)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    return 0;
}