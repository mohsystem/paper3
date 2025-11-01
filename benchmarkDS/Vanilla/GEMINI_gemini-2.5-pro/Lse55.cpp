/*
 * This code requires the header-only library 'cpp-httplib'.
 * You must download 'httplib.h' from its GitHub repository and place it
 * in the same directory or in your include path.
 *
 * GitHub: https://github.com/yhirose/cpp-httplib
 *
 * Compilation command:
 * g++ -o Lse55 Lse55.cpp -pthread -std=c++11
*/
#include <iostream>
#include <atomic>
#include "httplib.h"

// Simple in-memory atomic flags to simulate a session state
std::atomic<bool> isLoggedIn(false);
std::atomic<bool> isAdmin(false);

// Function to render the login page HTML
std::string login_page_content() {
    return R"(
        <html><body>
        <h2>Login Form</h2>
        <form action='/do_login' method='post'>
          Username: <input type='text' name='username'><br>
          Password: <input type='password' name='password'><br>
          <input type='submit' value='Login'>
        </form>
        </body></html>
    )";
}

// Function to handle the login logic
void do_login_action(const httplib::Request& req, httplib::Response& res) {
    std::string username;
    std::string password;

    if (req.has_param("username")) {
        username = req.get_param_value("username");
    }
    if (req.has_param("password")) {
        password = req.get_param_value("password");
    }

    if (username == "user" && password == "userpass") {
        isLoggedIn = true;
        isAdmin = false;
        res.set_redirect("/");
    } else if (username == "admin" && password == "adminpass") {
        isLoggedIn = true;
        isAdmin = true;
        res.set_redirect("/");
    } else {
        isLoggedIn = false;
        isAdmin = false;
        res.set_redirect("/login");
    }
}

// Function to render the index page
std::string index_page_content() {
    if (isLoggedIn) {
        return "<h1>Welcome to the Index Page!</h1><p>You are logged in.</p>";
    }
    return "<h1>Welcome to the Index Page!</h1><p>Please <a href='/login'>login</a>.</p>";
}

// Function to render the user page
std::string user_page_content() {
    if (isLoggedIn) {
        return "<h1>Welcome, User!</h1><p>This is the standard user page.</p>";
    }
    return "<h1>Access Denied</h1><p>You must be logged in to see this page.</p>";
}

// Function to render the admin page
std::string admin_page_content() {
    if (isLoggedIn && isAdmin) {
        return "<h1>Welcome, Admin!</h1><p>This is the admin-only page.</p>";
    }
    return "<h1>Access Denied</h1><p>You must be an admin to see this page.</p>";
}


class Lse55 {
public:
    static void run_server() {
        httplib::Server svr;

        // 1. The login function (GET request)
        svr.Get("/login", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(login_page_content(), "text/html");
        });

        // 2. The do_login function (POST request)
        svr.Post("/do_login", do_login_action);

        // 3. The index function (GET request)
        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(index_page_content(), "text/html");
        });

        // 4. The user_page function (GET request)
        svr.Get("/user", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(user_page_content(), "text/html");
        });

        // 5. The admin_page function (GET request)
        svr.Get("/admin", [](const httplib::Request&, httplib::Response& res) {
            res.set_content(admin_page_content(), "text/html");
        });

        std::cout << "Server started on http://localhost:8080" << std::endl;
        svr.listen("0.0.0.0", 8080);
    }
};

int main() {
    std::cout << "--- Test Cases (use curl in a separate terminal) ---" << std::endl;
    std::cout << "1. View login page: curl http://localhost:8080/login" << std::endl;
    std::cout << "2. Attempt login: curl -X POST -d \"username=admin&password=adminpass\" http://localhost:8080/do_login -v" << std::endl;
    std::cout << "3. View index page (after login): curl http://localhost:8080/" << std::endl;
    std::cout << "4. View user page (after login): curl http://localhost:8080/user" << std::endl;
    std::cout << "5. View admin page (after admin login): curl http://localhost:8080/admin" << std::endl;
    
    Lse55::run_server();
    return 0;
}