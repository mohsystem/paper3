#include <iostream>
#include <string>

static const std::string ADMIN_USERNAME = "admin";
static const std::string ADMIN_PASSWORD = "AdminPassword!234";

std::string login() {
    return std::string("<!DOCTYPE html>")
        + "<html><head><meta charset=\"utf-8\"><title>Login</title></head>"
        + "<body>"
        + "<form action=\"/do_login\" method=\"post\" autocomplete=\"off\">"
        + "<label for=\"username\">Username:</label>"
        + "<input type=\"text\" id=\"username\" name=\"username\" required minlength=\"1\" maxlength=\"64\" />"
        + "<label for=\"password\">Password:</label>"
        + "<input type=\"password\" id=\"password\" name=\"password\" required minlength=\"1\" maxlength=\"128\" />"
        + "<button type=\"submit\">Login</button>"
        + "</form>"
        + "</body></html>";
}

static bool valid_username(const std::string& u) {
    if (u.size() < 1 || u.size() > 64) return false;
    for (char c : u) {
        bool ok = (c >= 'A' && c <= 'Z')
               || (c >= 'a' && c <= 'z')
               || (c >= '0' && c <= '9')
               || c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static bool valid_password(const std::string& p) {
    return p.size() >= 1 && p.size() <= 128;
}

static bool constant_time_equals(const std::string& a, const std::string& b) {
    size_t maxlen = a.size() > b.size() ? a.size() : b.size();
    unsigned char diff = 0;
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char ai = i < a.size() ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char bi = i < b.size() ? static_cast<unsigned char>(b[i]) : 0;
        diff |= static_cast<unsigned char>(ai ^ bi);
    }
    return diff == 0 && a.size() == b.size();
}

std::string do_login(const std::string& username, const std::string& password) {
    if (!valid_username(username) || !valid_password(password)) {
        return "400 Bad Request";
    }
    bool isAdminUser = constant_time_equals(username, ADMIN_USERNAME);
    bool isAdminPass = constant_time_equals(password, ADMIN_PASSWORD);
    if (isAdminUser && isAdminPass) {
        return "REDIRECT:/admin_page";
    }
    return "401 Unauthorized";
}

int main() {
    // 5 test cases
    std::cout << "TEST1 FORM:\n" << login() << "\n";
    std::cout << "TEST2 ADMIN CORRECT:\n" << do_login("admin", "AdminPassword!234") << "\n";
    std::cout << "TEST3 ADMIN WRONG PASS:\n" << do_login("admin", "wrong") << "\n";
    std::cout << "TEST4 NON-ADMIN:\n" << do_login("user", "AdminPassword!234") << "\n";
    std::cout << "TEST5 BAD INPUT:\n" << do_login("", "") << "\n";
    return 0;
}