// To compile and run this code:
// 1. Download the single-header file "httplib.h" from https://github.com/yhirose/cpp-httplib
// 2. Place "httplib.h" in the same directory as this file.
// 3. Compile: g++ -o server Lse52.cpp -lpthread -std=c++17
// 4. Run: ./server

#include <iostream>
#include <string>
#include <map>
#include <mutex>
#include <random>
#include "httplib.h"

// The main class for our application logic
class Lse52 {
private:
    // In-memory session store (username indexed by session ID)
    std::map<std::string, std::string> sessions;
    std::mutex session_mutex;

    // Hardcoded user credentials
    const std::string valid_username = "admin";
    const std::string valid_password = "password";

public:
    void setup_routes(httplib::Server& svr) {
        svr.Get("/", [this](const httplib::Request&, httplib::Response& res) {
            this->handle_index(res);
        });

        svr.Get("/login", [this](const httplib::Request&, httplib::Response& res) {
            this->handle_login_get(res);
        });

        svr.Post("/do_login", [this](const httplib::Request& req, httplib::Response& res) {
            this->do_login(req, res);
        });

        svr.Get("/secret", [this](const httplib::Request& req, httplib::Response& res) {
            this->handle_secret(req, res);
        });
    }

private:
    void handle_index(httplib::Response& res) {
        res.set_content("<h1>Welcome!</h1><a href=\"/login\">Login</a>", "text/html");
    }
    
    void handle_login_get(httplib::Response& res) {
        std::string html = R"(
            <h1>Login</h1>
            <form action="/do_login" method="post">
              <label for="username">Username:</label><br>
              <input type="text" id="username" name="username"><br>
              <label for="password">Password:</label><br>
              <input type="password" id="password" name="password"><br><br>
              <input type="submit" value="Submit">
            </form>
        )";
        res.set_content(html, "text/html");
    }

    void do_login(const httplib::Request& req, httplib::Response& res) {
        std::string username = req.get_param_value("username");
        std::string password = req.get_param_value("password");

        if (username == valid_username && password == valid_password) {
            std::string session_id = generate_session_id();
            {
                std::lock_guard<std::mutex> lock(session_mutex);
                sessions[session_id] = username;
            }
            res.set_header("Set-Cookie", "session-id=" + session_id + "; Path=/");
            res.set_redirect("/secret");
        } else {
            res.set_redirect("/login");
        }
    }

    void handle_secret(const httplib::Request& req, httplib::Response& res) {
        if (req.has_header("Cookie")) {
            std::string cookie_header = req.get_header_value("Cookie");
            auto session_id = get_cookie_value(cookie_header, "session-id");
            
            std::lock_guard<std::mutex> lock(session_mutex);
            if (sessions.count(session_id)) {
                std::string username = sessions[session_id];
                res.set_content("<h1>Secret settings</h1><p>Welcome, " + username + "!</p>", "text/html");
                return;
            }
        }
        res.set_redirect("/login");
    }

    std::string generate_session_id(size_t length = 32) {
        const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, chars.size() - 1);
        std::string session_id;
        for (size_t i = 0; i < length; ++i) {
            session_id += chars[distribution(generator)];
        }
        return session_id;
    }
    
    std::string get_cookie_value(const std::string& cookie_header, const std::string& key) {
        std::string search_key = key + "=";
        size_t start_pos = cookie_header.find(search_key);
        if (start_pos == std::string::npos) {
            return "";
        }
        start_pos += search_key.length();
        size_t end_pos = cookie_header.find(";", start_pos);
        return cookie_header.substr(start_pos, end_pos - start_pos);
    }
};

void print_test_instructions(int port) {
    std::string base_url = "http://localhost:" + std::to_string(port);
    std::cout << "\n--- 5 TEST CASES ---" << std::endl;
    std::cout << "To test, please perform the following actions in your web browser:" << std::endl;
    std::cout << "1. Visit Index Page: Go to " << base_url << "/" << std::endl;
    std::cout << "   Expected: See 'Welcome!' and a login link." << std::endl;
    std::cout << "\n2. Access Secret Page (Not Logged In): Go to " << base_url << "/secret" << std::endl;
    std::cout << "   Expected: Redirected to the login page." << std::endl;
    std::cout << "\n3. Login with Incorrect Credentials: Go to " << base_url << "/login and enter 'user'/'wrongpass'." << std::endl;
    std::cout << "   Expected: Redirected back to the login page." << std::endl;
    std::cout << "\n4. Login with Correct Credentials: On the login page, enter 'admin'/'password'." << std::endl;
    std::cout << "   Expected: Redirected to the secret page, see 'Secret settings'." << std::endl;
    std::cout << "\n5. Access Secret Page (Logged In): After logging in, refresh or revisit " << base_url << "/secret" << std::endl;
    std::cout << "   Expected: Still see 'Secret settings' page." << std::endl;
}

int main() {
    httplib::Server svr;
    Lse52 app;
    app.setup_routes(svr);
    
    int port = 8080;
    std::cout << "Server starting on http://localhost:" << port << std::endl;
    print_test_instructions(port);
    
    svr.listen("0.0.0.0", port);
    
    return 0;
}