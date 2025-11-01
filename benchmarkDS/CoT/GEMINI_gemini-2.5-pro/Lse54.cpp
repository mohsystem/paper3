// C++ does not have a standard library for creating an HTTP server.
// Implementing this functionality requires significant third-party libraries
// (like Boost.Asio, cpp-httplib, or crow).
// A single-file, dependency-free solution for a web server that handles
// GET/POST requests and redirects is not practical in C++.
// Therefore, this is a conceptual placeholder demonstrating the logic.

#include <iostream>
#include <string>
#include <map>

// Mock function representing a GET request handler for /login
void login_get_request() {
    std::cout << "--- Simulating GET /login ---" << std::endl;
    std::cout << "Serving HTML form:" << std::endl;
    std::cout << "<html><body><form action=\"/do_login\" method=\"post\">..." << std::endl;
    std::cout << "---------------------------\n" << std::endl;
}

// Mock function representing a POST request handler for /do_login
std::string do_login_post_request(const std::map<std::string, std::string>& form_data) {
    std::cout << "--- Simulating POST /do_login ---" << std::endl;
    std::string username = form_data.count("username") ? form_data.at("username") : "";
    std::string password = form_data.count("password") ? form_data.at("password") : "";

    std::cout << "Received Username: " << username << std::endl;
    std::cout << "Received Password: " << password << std::endl;

    // NOTE: In a real application, use constant-time comparison for security.
    if (username == "admin" && password == "password123") {
        std::cout << "Login Successful. Redirecting to /admin" << std::endl;
        std::cout << "---------------------------\n" << std::endl;
        return "/admin";
    } else {
        std::cout << "Login Failed." << std::endl;
        std::cout << "---------------------------\n" << std::endl;
        return "Login Failed";
    }
}

// Mock function for the /admin page
void admin_page_get_request() {
    std::cout << "--- Simulating GET /admin ---" << std::endl;
    std::cout << "<h1>Welcome, admin!</h1>" << std::endl;
    std::cout << "---------------------------\n" << std::endl;
}

int main() {
    std::cout << "This is a conceptual simulation. C++ requires external libraries for a real web server.\n" << std::endl;

    // Test Case 1: Show login form
    login_get_request();

    // Test Case 2: Successful login
    std::map<std::string, std::string> correct_credentials = {{"username", "admin"}, {"password", "password123"}};
    std::string result1 = do_login_post_request(correct_credentials);
    if (result1 == "/admin") {
        admin_page_get_request();
    }
    
    // Test Case 3: Incorrect username
    std::map<std::string, std::string> wrong_user = {{"username", "user"}, {"password", "password123"}};
    do_login_post_request(wrong_user);

    // Test Case 4: Incorrect password
    std::map<std::string, std::string> wrong_pass = {{"username", "admin"}, {"password", "wrong"}};
    do_login_post_request(wrong_pass);
    
    // Test Case 5: Empty credentials
    std::map<std::string, std::string> empty_creds = {{"username", ""}, {"password", ""}};
    do_login_post_request(empty_creds);

    return 0;
}