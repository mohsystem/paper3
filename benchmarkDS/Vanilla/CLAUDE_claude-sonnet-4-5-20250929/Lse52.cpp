
// C++ web application using CGI or similar framework
// This is a simplified example using iostream for demonstration
// In practice, you would use a web framework like Crow, Pistache, or Boost.Beast

#include <iostream>
#include <string>
#include <map>
#include <cstdlib>

class Lse52 {
private:
    std::map<std::string, std::string> session;
    
    std::string getIndexPage() {
        return "<html><body>"
               "<h1>Welcome to Index Page</h1>"
               "<a href='/login'>Go to Login</a>"
               "</body></html>";
    }
    
    std::string getLoginPage() {
        return "<html><body>"
               "<h1>Login Page</h1>"
               "<form action='/do_login' method='post'>"
               "Username: <input type='text' name='username'><br>"
               "Password: <input type='password' name='password'><br>"
               "<input type='submit' value='Login'>"
               "</form>"
               "</body></html>";
    }
    
    std::string getSecretPage(bool authenticated) {
        if (authenticated) {
            return "<html><body>"
                   "<h1>Secret settings</h1>"
                   "</body></html>";
        } else {
            return "<html><body>"
                   "<h1>Access Denied</h1>"
                   "<a href='/login'>Please login</a>"
                   "</body></html>";
        }
    }
    
public:
    bool doLogin(const std::string& username, const std::string& password) {
        if (username == "admin" && password == "password123") {
            session["username"] = username;
            return true;
        }
        return false;
    }
    
    bool isAuthenticated() {
        return session.find("username") != session.end();
    }
    
    void handleRequest(const std::string& path, const std::string& method,
                      const std::string& username = "", const std::string& password = "") {
        if (path == "/" && method == "GET") {
            std::cout << getIndexPage() << std::endl;
        } else if (path == "/login" && method == "GET") {
            std::cout << getLoginPage() << std::endl;
        } else if (path == "/do_login" && method == "POST") {
            if (doLogin(username, password)) {
                std::cout << "Redirect to /secret" << std::endl;
            } else {
                std::cout << "Redirect to /login" << std::endl;
            }
        } else if (path == "/secret" && method == "GET") {
            std::cout << getSecretPage(isAuthenticated()) << std::endl;
        }
    }
};

int main() {
    Lse52 app;
    
    std::cout << "Test Case 1: Access Index Page" << std::endl;
    app.handleRequest("/", "GET");
    std::cout << std::endl;
    
    std::cout << "Test Case 2: Access Login Page" << std::endl;
    app.handleRequest("/login", "GET");
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Valid Login" << std::endl;
    app.handleRequest("/do_login", "POST", "admin", "password123");
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Access Secret Page (Authenticated)" << std::endl;
    app.handleRequest("/secret", "GET");
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Invalid Login" << std::endl;
    Lse52 app2;
    app2.handleRequest("/do_login", "POST", "wrong", "wrong");
    std::cout << std::endl;
    
    return 0;
}
