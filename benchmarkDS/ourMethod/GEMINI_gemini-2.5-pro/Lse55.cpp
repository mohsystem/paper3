// This code requires the cpp-httplib library.
// Download "httplib.h" from https://github.com/yhirose/cpp-httplib
// and place it in the same directory as this file.
// To compile: g++ -o server server.cpp -I. -lpthread -lssl -lcrypto
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "httplib.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// GET /login
void get_login(const httplib::Request& req, httplib::Response& res) {
    std::string body = R"(
        <h1>Login Page</h1>
        <form action="/do_login" method="post">
          Username: <input type="text" name="username"><br>
          Password: <input type="password" name="password"><br>
          <input type="submit" value="Login">
        </form>
    )";
    res.set_content(body, "text/html");
}

// POST /do_login
void post_do_login(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    std::string password;
    
    // Rules#6: Ensure all input is validated
    if (req.has_param("username") && req.has_param("password")) {
        username = req.get_param_value("username");
        password = req.get_param_value("password");
    } else {
        res.status = 400; // Bad Request
        res.set_content("<h1>Username and password are required.</h1>", "text/html");
        return;
    }
    
    if (username.empty() || password.empty()) {
        res.status = 400; // Bad Request
        res.set_content("<h1>Username and password cannot be empty.</h1>", "text/html");
        return;
    }

    // In a real application, you would validate credentials against a database.
    // Rules#1, #3, #4, #5: NEVER store passwords in plaintext.
    // Use a strong, salted hashing library like Argon2.
    // The following is for demonstration purposes ONLY and is highly insecure.
    if (username == "admin" && password == "password123") {
        res.set_content("<h1>Admin Login Successful!</h1>", "text/html");
    } else if (username == "user" && password == "password123") {
        res.set_content("<h1>User Login Successful!</h1>", "text/html");
    } else {
        res.status = 401; // Unauthorized
        res.set_content("<h1>Login Failed: Invalid credentials.</h1>", "text/html");
    }
}

// GET /
void get_index(const httplib::Request& req, httplib::Response& res) {
    res.set_content("<h1>Welcome to the Index Page!</h1>", "text/html");
}

// GET /user
void get_user_page(const httplib::Request& req, httplib::Response& res) {
    res.set_content("<h1>Welcome to the User Page!</h1>", "text/html");
}

// GET /admin
void get_admin_page(const httplib::Request& req, httplib::Response& res) {
    res.set_content("<h1>Welcome to the Admin Page!</h1>", "text/html");
}

void run_tests() {
    httplib::Client cli("localhost", 8080);
    cli.set_connection_timeout(5);

    std::cout << "\n--- Running 5 Test Cases ---" << std::endl;
    
    // Test Case 1: index function
    std::cout << "Test Case 1: GET /index" << std::endl;
    if (auto res = cli.Get("/index")) {
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body.substr(0, 100) << "..." << std::endl;
    } else {
        std::cerr << "Error: " << httplib::to_string(res.error()) << std::endl;
    }
    std::cout << "----------------------------" << std::endl;
    
    // Test Case 2: login function
    std::cout << "Test Case 2: GET /login" << std::endl;
    if (auto res = cli.Get("/login")) {
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body.substr(0, 100) << "..." << std::endl;
    } else {
        std::cerr << "Error: " << httplib::to_string(res.error()) << std::endl;
    }
    std::cout << "----------------------------" << std::endl;

    // Test Case 3: do_login function
    std::cout << "Test Case 3: POST /do_login" << std::endl;
    httplib::Params params{{"username", "admin"}, {"password", "password123"}};
    if (auto res = cli.Post("/do_login", params)) {
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body.substr(0, 100) << "..." << std::endl;
    } else {
        std::cerr << "Error: " << httplib::to_string(res.error()) << std::endl;
    }
    std::cout << "----------------------------" << std::endl;

    // Test Case 4: user_page function
    std::cout << "Test Case 4: GET /user" << std::endl;
    if (auto res = cli.Get("/user")) {
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body.substr(0, 100) << "..." << std::endl;
    } else {
        std::cerr << "Error: " << httplib::to_string(res.error()) << std::endl;
    }
    std::cout << "----------------------------" << std::endl;

    // Test Case 5: admin_page function
    std::cout << "Test Case 5: GET /admin" << std::endl;
    if (auto res = cli.Get("/admin")) {
        std::cout << "Status: " << res->status << std::endl;
        std::cout << "Body: " << res->body.substr(0, 100) << "..." << std::endl;
    } else {
        std::cerr << "Error: " << httplib::to_string(res.error()) << std::endl;
    }
    std::cout << "--- Test Cases Finished ---" << std::endl;
}

int main(void) {
    httplib::Server svr;

    svr.Get("/", get_index);
    svr.Get("/index", get_index);
    svr.Get("/login", get_login);
    svr.Post("/do_login", post_do_login);
    svr.Get("/user", get_user_page);
    svr.Get("/admin", get_admin_page);

    std::thread server_thread([&]() {
        std::cout << "Starting server on port 8080..." << std::endl;
        if (!svr.listen("0.0.0.0", 8080)) {
            std::cerr << "Failed to start server" << std::endl;
        }
    });

    // Give the server a moment to start up
    std::this_thread::sleep_for(std::chrono::seconds(1));

    run_tests();

    std::cout << "Stopping server..." << std::endl;
    svr.stop();
    server_thread.join();
    std::cout << "Server stopped." << std::endl;

    return 0;
}