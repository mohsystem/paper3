#include <iostream>
#include <string>

class Lse54 {
public:
    static std::string login() {
        return R"(<!DOCTYPE html>
<html>
<head><title>Login</title></head>
<body>
  <h1>Login</h1>
  <form method="POST" action="/do_login">
    <label>Username: <input type="text" name="username"/></label><br/>
    <label>Password: <input type="password" name="password"/></label><br/>
    <button type="submit">Login</button>
  </form>
</body>
</html>)";
    }

    static std::string do_login(const std::string& username, const std::string& password) {
        if (username == "admin") {
            return "REDIRECT:/admin_page";
        }
        return login();
    }

    static std::string admin_page() {
        return R"(<!DOCTYPE html>
<html>
<head><title>Admin</title></head>
<body><h1>Welcome, Admin!</h1></body>
</html>)";
    }
};

int main() {
    // Test 1: GET /login
    std::cout << "Test 1 - GET /login:" << std::endl;
    std::cout << Lse54::login() << std::endl;
    std::cout << "-----" << std::endl;

    // Test 2: POST /do_login with admin user
    std::cout << "Test 2 - POST /do_login (admin, 1234):" << std::endl;
    std::string res2 = Lse54::do_login("admin", "1234");
    std::cout << res2 << std::endl;
    if (res2.rfind("REDIRECT:/admin_page", 0) == 0) {
        std::cout << Lse54::admin_page() << std::endl;
    }
    std::cout << "-----" << std::endl;

    // Test 3: POST /do_login with non-admin user
    std::cout << "Test 3 - POST /do_login (user, pass):" << std::endl;
    std::string res3 = Lse54::do_login("user", "pass");
    std::cout << res3 << std::endl;
    std::cout << "-----" << std::endl;

    // Test 4: POST /do_login with Admin (case-sensitive fail)
    std::cout << "Test 4 - POST /do_login (Admin, 1234):" << std::endl;
    std::string res4 = Lse54::do_login("Admin", "1234");
    std::cout << res4 << std::endl;
    std::cout << "-----" << std::endl;

    // Test 5: POST /do_login with admin and empty password
    std::cout << "Test 5 - POST /do_login (admin, ''):" << std::endl;
    std::string res5 = Lse54::do_login("admin", "");
    std::cout << res5 << std::endl;
    if (res5.rfind("REDIRECT:/admin_page", 0) == 0) {
        std::cout << Lse54::admin_page() << std::endl;
    }
    std::cout << "-----" << std::endl;

    return 0;
}