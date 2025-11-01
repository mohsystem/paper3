/*
To compile and run this code:
1. Download the single-header file "httplib.h" from the yhirose/cpp-httplib GitHub repository.
   (https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h)
2. Place "httplib.h" in the same directory as this file.
3. Compile with: g++ -o server Lse54.cpp -pthread
4. Run with: ./server
*/
#include <iostream>
#include "httplib.h"

// The login function (GET request handler)
void login(const httplib::Request& req, httplib::Response& res) {
    std::string form_html = R"(
    <html>
        <body>
            <h2>Login</h2>
            <form method="POST" action="/do_login">
                Username: <input type="text" name="username"><br>
                Password: <input type="password" name="password"><br>
                <input type="submit" value="Login">
            </form>
        </body>
    </html>
    )";
    res.set_content(form_html, "text/html");
}

// The do_login function (POST request handler)
void do_login(const httplib::Request& req, httplib::Response& res) {
    if (req.has_param("username") && req.has_param("password")) {
        std::string username = req.get_param_value("username");
        std::string password = req.get_param_value("password");

        if (username == "admin" && password == "admin") {
            res.set_redirect("/admin_page");
        } else {
            res.status = 401; // Unauthorized
            res.set_content("Login Failed: Invalid credentials.", "text/plain");
        }
    } else {
        res.status = 400; // Bad Request
        res.set_content("Username and password are required.", "text/plain");
    }
}

// Handler for the admin page
void admin_page(const httplib::Request& req, httplib::Response& res) {
    res.set_content("<html><body><h1>Welcome, Admin!</h1></body></html>", "text/html");
}

int main(void) {
    httplib::Server svr;

    svr.Get("/login", login);
    svr.Post("/do_login", do_login);
    svr.Get("/admin_page", admin_page);

    int port = 8081;
    std::cout << "C++ server started on port " << port << std::endl;
    std::cout << "--- 5 Test Cases ---" << std::endl;
    std::cout << "1. View login form: Open http://localhost:" << port << "/login in a browser or run: curl http://localhost:" << port << "/login" << std::endl;
    std::cout << "2. Successful login: curl -X POST -d \"username=admin&password=admin\" -L http://localhost:" << port << "/do_login" << std::endl;
    std::cout << "3. Failed login (wrong user): curl -X POST -d \"username=user&password=admin\" http://localhost:" << port << "/do_login" << std::endl;
    std::cout << "4. Failed login (wrong pass): curl -X POST -d \"username=admin&password=wrong\" http://localhost:" << port << "/do_login" << std::endl;
    std::cout << "5. View admin page directly: curl http://localhost:" << port << "/admin_page" << std::endl;

    svr.listen("0.0.0.0", port);
    
    return 0;
}