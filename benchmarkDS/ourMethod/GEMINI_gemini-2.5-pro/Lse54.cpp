#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

// This requires the httplib.h header file.
// Download from: https://github.com/yhirose/cpp-httplib
// Ensure the file is in your include path or the same directory.
#include "httplib.h" 

// In a real application, never hardcode credentials.
// CWE-798: Use of Hard-coded Credentials
// Use a secure identity provider and retrieve secrets from a vault or environment variables.
// This example uses plaintext comparison for simplicity, but production systems
// MUST use a strong, salted hashing algorithm like Argon2, scrypt, or PBKDF2.
// CWE-759: Use of a One-Way Hash without a Salt
const std::string ADMIN_USERNAME = "admin";
const std::string ADMIN_PASSWORD = "password";

// Basic HTML escaping to mitigate CWE-79 (XSS)
std::string escapeHtml(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&':  output += "&amp;";       break;
            case '\"': output += "&quot;";      break;
            case '\'': output += "&#x27;";      break;
            case '<':  output += "&lt;";        break;
            case '>':  output += "&gt;";        break;
            default:   output += c;             break;
        }
    }
    return output;
}

std::string get_login_form(const std::string& error_message = "") {
    std::string error_html;
    if (!error_message.empty()) {
        error_html = "<p style='color:red;'>" + escapeHtml(error_message) + "</p>";
    }
    return R"(
    <!DOCTYPE html>
    <html>
    <head><title>Login</title></head>
    <body>
        <h1>Login</h1>
        )" + error_html + R"(
        <form method="POST" action="/login">
            Username: <input type="text" name="username"><br>
            Password: <input type="password" name="password"><br>
            <input type="submit" value="Login">
        </form>
    </body>
    </html>
    )";
}

// Constant-time string comparison to mitigate timing attacks
bool secure_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

void login_get(const httplib::Request& req, httplib::Response& res) {
    res.set_content(get_login_form(), "text/html; charset=UTF-8");
}

void do_login_post(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    std::string password;
    
    // Rule #1: Ensure input exists and is validated.
    if (req.has_param("username")) {
        username = req.get_param_value("username");
    }
    if (req.has_param("password")) {
        password = req.get_param_value("password");
    }

    if (secure_compare(username, ADMIN_USERNAME) && secure_compare(password, ADMIN_PASSWORD)) {
        res.set_redirect("/admin");
    } else {
        // Add a small random delay to make timing attacks harder
        std::this_thread::sleep_for(std::chrono::milliseconds(50 + (rand() % 50)));
        res.status = 401; // Unauthorized
        res.set_content(get_login_form("Invalid username or password."), "text/html; charset=UTF-8");
    }
}

void admin_page_get(const httplib::Request& req, httplib::Response& res) {
    const std::string body = "<html><body><h1>Welcome, Admin!</h1></body></html>";
    res.set_content(body, "text/html; charset=UTF-8");
}

int main() {
    httplib::Server svr;

    svr.Get("/login", login_get);
    svr.Post("/login", do_login_post);
    svr.Get("/admin", admin_page_get);

    int port = 8080;
    std::cout << "Server starting on http://localhost:" << port << std::endl;
    std::cout << "--- Test Cases ---" << std::endl;
    std::cout << "1. View login form: curl http://localhost:8080/login" << std::endl;
    std::cout << "2. Successful login (will get a 302 redirect): curl -i -L -d \"username=admin&password=password\" http://localhost:8080/login" << std::endl;
    std::cout << "3. Failed login (wrong password): curl -i -d \"username=admin&password=wrong\" http://localhost:8080/login" << std::endl;
    std::cout << "4. Failed login (wrong username): curl -i -d \"username=user&password=password\" http://localhost:8080/login" << std::endl;
    std::cout << "5. Access admin page after successful login: Follow the redirect from test case #2." << std::endl;

    // Production systems MUST use HTTPS. This requires compiling with CPPHTTPLIB_OPENSSL_SUPPORT
    // and providing certificate and key files.
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }
    
    return 0;
}