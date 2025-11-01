// This code requires the cpp-httplib library.
// Download `httplib.h` from https://github.com/yhirose/cpp-httplib
// and place it in the same directory or in your include path.
// Compile with: g++ -o server Lse55.cpp -pthread -std=c++17
#define CPPHTTPLIB_OPENSSL_SUPPORT // For HTTPS, if needed. Requires OpenSSL libs.

#include <iostream>
#include <map>
#include <string>
#include <mutex>
#include <random>
#include <sstream>
#include "httplib.h"

// --- Data Structures and Storage (In-Memory) ---

struct User {
    std::string password; // In a real app, this should be a secure hash.
    std::string role;
};

struct Session {
    std::string username;
    std::string role;
    std::string csrf_token;
};

// Global, thread-safe stores. In a real app, use a database or a proper cache like Redis.
std::map<std::string, User> g_users;
std::map<std::string, Session> g_sessions;
std::mutex g_session_mutex;

// --- Utility Functions ---

// Generates a random hex string for tokens
std::string generate_token() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << distrib(gen);
    }
    return ss.str();
}

// Simple HTML escaping
std::string html_escape(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}


// --- Main Application Logic ---

// The main class name is not applicable in this C++ structure but we follow the naming for the file.
class Lse55 {
public:
    static void run() {
        httplib::Server svr;

        // Populate users
        g_users["user"] = {"password123", "USER"};
        g_users["admin"] = {"adminpass", "ADMIN"};

        // 1. Index function (GET)
        svr.Get("/", [](const httplib::Request &req, httplib::Response &res) {
            auto session_id = req.get_header_value("Cookie");
            if (session_id.rfind("sessionId=", 0) == 0) {
                session_id = session_id.substr(10);
            } else {
                session_id = "";
            }

            std::lock_guard<std::mutex> lock(g_session_mutex);
            if (!session_id.empty() && g_sessions.count(session_id) && !g_sessions[session_id].username.empty()) {
                auto& session = g_sessions[session_id];
                std::string body = "<h1>Welcome, " + html_escape(session.username) + "!</h1><p><a href='/user'>User Page</a></p>";
                res.set_content(body, "text/html");
            } else {
                res.set_content("<h1>Welcome!</h1><p>Please <a href='/login'>login</a>.</p>", "text/html");
            }
        });

        // 2. Login function (GET)
        svr.Get("/login", [](const httplib::Request &req, httplib::Response &res) {
            std::string session_id = generate_token();
            std::string csrf_token = generate_token();
            {
                std::lock_guard<std::mutex> lock(g_session_mutex);
                g_sessions[session_id] = {"", "", csrf_token};
            }
            res.set_header("Set-Cookie", "sessionId=" + session_id + "; Path=/; HttpOnly; SameSite=Lax");
            std::string form = R"(
                <form action='/do_login' method='post'>
                    <h2>Login</h2>
                    <label for='username'>Username:</label><br>
                    <input type='text' id='username' name='username'><br>
                    <label for='password'>Password:</label><br>
                    <input type='password' id='password' name='password'><br>
                    <input type='hidden' name='csrf_token' value=')" + csrf_token + R"('>
                    <br><input type='submit' value='Login'>
                </form>
            )";
            res.set_content(form, "text/html");
        });

        // 3. do_login function (POST)
        svr.Post("/do_login", [](const httplib::Request &req, httplib::Response &res) {
            auto username = req.get_param_value("username");
            auto password = req.get_param_value("password");
            auto form_csrf = req.get_param_value("csrf_token");

            auto cookie_header = req.get_header_value("Cookie");
            auto session_id = cookie_header.substr(cookie_header.find("sessionId=") + 10);

            std::string expected_csrf;
            {
                std::lock_guard<std::mutex> lock(g_session_mutex);
                if (g_sessions.count(session_id)) {
                    expected_csrf = g_sessions[session_id].csrf_token;
                }
                g_sessions.erase(session_id); // Invalidate old session
            }

            if (expected_csrf.empty() || expected_csrf != form_csrf) {
                res.status = 403;
                res.set_content("Forbidden: Invalid CSRF Token", "text/plain");
                return;
            }

            if (g_users.count(username) && g_users[username].password == password) {
                std::string new_session_id = generate_token();
                {
                    std::lock_guard<std::mutex> lock(g_session_mutex);
                    g_sessions[new_session_id] = {username, g_users[username].role, generate_token()};
                }
                res.set_header("Set-Cookie", "sessionId=" + new_session_id + "; Path=/; HttpOnly; SameSite=Lax");
                res.set_redirect("/");
            } else {
                res.set_redirect("/login");
            }
        });

        // 4. user_page function (GET)
        svr.Get("/user", [](const httplib::Request &req, httplib::Response &res) {
            auto session_id = req.get_header_value("Cookie");
            if (session_id.rfind("sessionId=", 0) == 0) {
                 session_id = session_id.substr(10);
            } else {
                res.set_redirect("/login");
                return;
            }

            std::lock_guard<std::mutex> lock(g_session_mutex);
            if (g_sessions.count(session_id) && !g_sessions[session_id].username.empty()) {
                auto& session = g_sessions[session_id];
                std::string body = "<h1>User Page</h1><p>Hello, " + html_escape(session.username) + ". Your role is " + html_escape(session.role) + ".</p>";
                res.set_content(body, "text/html");
            } else {
                res.set_redirect("/login");
            }
        });
        
        // 5. admin_page function (GET)
        svr.Get("/admin", [](const httplib::Request &req, httplib::Response &res) {
            auto session_id = req.get_header_value("Cookie");
            if (session_id.rfind("sessionId=", 0) == 0) {
                session_id = session_id.substr(10);
            } else {
                res.set_redirect("/login");
                return;
            }

            std::lock_guard<std::mutex> lock(g_session_mutex);
            if (g_sessions.count(session_id) && g_sessions[session_id].role == "ADMIN") {
                res.set_content("<h1>Admin Page</h1><p>Welcome, administrator!</p>", "text/html");
            } else if (g_sessions.count(session_id)) {
                res.status = 403;
                res.set_content("<h1>403 Forbidden</h1>", "text/html");
            } else {
                res.set_redirect("/login");
            }
        });

        std::cout << "Server started on http://localhost:8080" << std::endl;
        std::cout << "--- Test Cases ---" << std::endl;
        std::cout << "1. Visit the index page: http://localhost:8080/" << std::endl;
        std::cout << "2. Visit the login page: http://localhost:8080/login" << std::endl;
        std::cout << "3. Login as 'user' with password 'password123' via the form." << std::endl;
        std::cout << "4. After logging in, visit the user page: http://localhost:8080/user" << std::endl;
        std::cout << "5. Try to visit the admin page (will be forbidden): http://localhost:8080/admin" << std::endl;
        std::cout << "   Then, logout (by clearing cookies) and log in as 'admin' to access it." << std::endl;

        svr.listen("0.0.0.0", 8080);
    }
};

int main() {
    Lse55::run();
    return 0;
}