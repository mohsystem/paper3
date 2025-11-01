#include <iostream>
#include <string>

static bool constant_time_equals(const std::string& a, const std::string& b) {
    size_t maxlen = (a.size() > b.size()) ? a.size() : b.size();
    unsigned char result = 0;
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char ca = (i < a.size()) ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char cb = (i < b.size()) ? static_cast<unsigned char>(b[i]) : 0;
        result |= (ca ^ cb);
    }
    return (result == 0) && (a.size() == b.size());
}

std::string login() {
    return "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Login</title></head>"
           "<body><form method=\"POST\" action=\"/do_login\">"
           "<label>Username: <input type=\"text\" name=\"username\" required></label><br>"
           "<label>Password: <input type=\"password\" name=\"password\" required></label><br>"
           "<button type=\"submit\">Login</button>"
           "</form></body></html>";
}

std::string do_login(const std::string& username, const std::string& password) {
    (void)password; // not used for admin check per specification
    if (constant_time_equals(username, "admin")) {
        return "REDIRECT:/admin_page";
    }
    return "";
}

int main() {
    std::cout << "GET /login ->\n" << login() << "\n\n";

    // 5 test cases
    std::cout << "POST /do_login admin/adminpass -> " << do_login("admin", "adminpass") << "\n";
    std::cout << "POST /do_login admin/wrong -> " << do_login("admin", "wrong") << "\n";
    std::cout << "POST /do_login user/adminpass -> " << do_login("user", "adminpass") << "\n";
    std::cout << "POST /do_login empty/empty -> " << do_login("", "") << "\n";
    std::cout << "POST /do_login Admin(case)/pass -> " << do_login("Admin", "pass") << "\n";
    return 0;
}