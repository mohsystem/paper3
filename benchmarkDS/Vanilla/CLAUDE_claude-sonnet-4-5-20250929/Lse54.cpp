
#include <iostream>
#include <string>
#include <map>

class Lse54 {
public:
    // GET request - display login form
    static std::string login() {
        std::string form = 
            "<html><body>\\n"
            "<h2>Login Form</h2>\\n"
            "<form method='POST' action='/do_login'>\\n"
            "Username: <input type='text' name='username'><br>\\n"
            "Password: <input type='password' name='password'><br>\\n"
            "<input type='submit' value='Login'>\\n"
            "</form>\\n"
            "</body></html>";
        return form;
    }
    
    // POST request - process login
    static std::string do_login(const std::string& username, const std::string& password) {
        if (!username.empty() && username == "admin") {
            return "REDIRECT:/admin_page";
        } else {
            return "<html><body><h3>Login failed or user is not admin</h3></body></html>";
        }
    }
    
    static std::string admin_page() {
        return "<html><body><h2>Welcome to Admin Page</h2></body></html>";
    }
};

int main() {
    // Test case 1: Admin login
    std::cout << "Test 1 - Admin login:" << std::endl;
    std::string result1 = Lse54::do_login("admin", "password123");
    std::cout << result1 << std::endl << std::endl;
    
    // Test case 2: Non-admin login
    std::cout << "Test 2 - Non-admin login:" << std::endl;
    std::string result2 = Lse54::do_login("user", "password123");
    std::cout << result2 << std::endl << std::endl;
    
    // Test case 3: Empty username
    std::cout << "Test 3 - Empty username:" << std::endl;
    std::string result3 = Lse54::do_login("", "password123");
    std::cout << result3 << std::endl << std::endl;
    
    // Test case 4: Admin with different password
    std::cout << "Test 4 - Admin with wrong password:" << std::endl;
    std::string result4 = Lse54::do_login("admin", "wrong");
    std::cout << result4 << std::endl << std::endl;
    
    // Test case 5: Get login form
    std::cout << "Test 5 - Display login form:" << std::endl;
    std::string result5 = Lse54::login();
    std::cout << result5 << std::endl;
    
    return 0;
}
