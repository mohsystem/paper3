
#include <iostream>
#include <string>
#include <cstring>

class Lse54 {
public:
    static std::string login() {
        std::string form = 
            "<!DOCTYPE html>\\n"
            "<html>\\n"
            "<head><title>Login</title></head>\\n"
            "<body>\\n"
            "    <h2>Login Form</h2>\\n"
            "    <form action='/do_login' method='post'>\\n"
            "        Username: <input type='text' name='username' required><br><br>\\n"
            "        Password: <input type='password' name='password' required><br><br>\\n"
            "        <input type='submit' value='Login'>\\n"
            "    </form>\\n"
            "</body>\\n"
            "</html>";
        return form;
    }
    
    static std::string do_login(const std::string& username, const std::string& password) {
        if (!username.empty() && !password.empty()) {
            if (isAdmin(username, password)) {
                return "redirect:/admin_page";
            } else {
                return "<h3>Access Denied</h3><a href='/login'>Try Again</a>";
            }
        }
        return "<h3>Invalid input</h3><a href='/login'>Try Again</a>";
    }
    
private:
    static bool isAdmin(const std::string& username, const std::string& password) {
        return (username == "admin" && password == "admin123");
    }
};

int main() {
    std::cout << "Test Case 1: Valid admin credentials" << std::endl;
    std::cout << "Result: " << Lse54::do_login("admin", "admin123") << std::endl;
    
    std::cout << "\\nTest Case 2: Invalid username" << std::endl;
    std::cout << "Result: " << Lse54::do_login("user", "admin123") << std::endl;
    
    std::cout << "\\nTest Case 3: Invalid password" << std::endl;
    std::cout << "Result: " << Lse54::do_login("admin", "wrongpass") << std::endl;
    
    std::cout << "\\nTest Case 4: Empty credentials" << std::endl;
    std::cout << "Result: " << Lse54::do_login("", "") << std::endl;
    
    std::cout << "\\nTest Case 5: Another invalid case" << std::endl;
    std::cout << "Result: " << Lse54::do_login("test", "test123") << std::endl;
    
    return 0;
}
