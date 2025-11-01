#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <vector>

static const int MIN_USERNAME_LEN = 1;
static const int MAX_USERNAME_LEN = 64;
static const int MIN_PASSWORD_LEN = 12;
static const int MAX_PASSWORD_LEN = 128;

static bool validUsername(const std::string& u) {
    if (u.size() < (size_t)MIN_USERNAME_LEN || u.size() > (size_t)MAX_USERNAME_LEN) return false;
    for (unsigned char c : u) {
        if (c > 0x7F) return false;
        if (!(std::isalnum(c) || c == '_' || c == '-' || c == '.' || c == '@')) return false;
    }
    return true;
}

static bool validPasswordPolicy(const std::string& p) {
    if (p.size() < (size_t)MIN_PASSWORD_LEN || p.size() > (size_t)MAX_PASSWORD_LEN) return false;
    bool hasU = false, hasL = false, hasD = false, hasS = false;
    std::string specials = "!@#$%^&*()_+-=[]{}|;:',.<>/?`~\"\\";
    for (unsigned char c : p) {
        if (c > 0x7F) return false;
        if (std::isupper(c)) hasU = true;
        else if (std::islower(c)) hasL = true;
        else if (std::isdigit(c)) hasD = true;
        else {
            bool ok = specials.find((char)c) != std::string::npos;
            if (!ok) return false;
            hasS = true;
        }
    }
    return hasU && hasL && hasD && hasS;
}

static bool constTimeEq(const std::string& a, const std::string& b) {
    size_t la = a.size(), lb = b.size();
    size_t len = la < lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < len; ++i) {
        diff |= (unsigned char)(a[i] ^ b[i]);
    }
    return diff == 0;
}

static bool set_env(const std::string& key, const std::string& val) {
#if defined(_WIN32)
    std::string kv = key + "=" + val;
    return _putenv(kv.c_str()) == 0;
#else
    // Use putenv for portability without depending on POSIX setenv strictly
    std::string kv = key + "=" + val;
    char* buf = (char*)std::malloc(kv.size() + 1);
    if (!buf) return false;
    std::memcpy(buf, kv.c_str(), kv.size() + 1);
    // putenv takes ownership of the buffer; we intentionally do not free it.
    return putenv(buf) == 0;
#endif
}

std::string login() {
    return "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Login</title></head>"
           "<body><h1>Login</h1>"
           "<form method=\"POST\" action=\"/do_login\">"
           "<label>Username: <input type=\"text\" name=\"username\" maxlength=\"64\" required></label><br>"
           "<label>Password: <input type=\"password\" name=\"password\" minlength=\"12\" maxlength=\"128\" required></label><br>"
           "<button type=\"submit\">Login</button>"
           "</form></body></html>";
}

std::string do_login(const std::string& username, const std::string& password) {
    if (!validUsername(username) || !validPasswordPolicy(password)) {
        return "INVALID_INPUT";
    }
    const char* envUser = std::getenv("ADMIN_USER");
    std::string adminUser = (envUser && *envUser) ? std::string(envUser) : "admin";
    const char* envPass = std::getenv("ADMIN_PASSWORD");
    if (envPass == nullptr || *envPass == '\0') {
        return "SERVER_NOT_CONFIGURED";
    }
    std::string adminPass(envPass);
    if (!validPasswordPolicy(adminPass)) {
        return "SERVER_NOT_CONFIGURED";
    }
    if (!constTimeEq(username, adminUser)) {
        return "ACCESS_DENIED";
    }
    if (constTimeEq(password, adminPass)) {
        return "REDIRECT:/admin_page";
    }
    return "ACCESS_DENIED";
}

int main() {
    // Configure environment for testing
    set_env("ADMIN_USER", "admin");
    set_env("ADMIN_PASSWORD", "StrongPassw0rd!X");

    std::cout << "TEST1 login():\n" << login() << "\n\n";

    std::cout << "TEST2 do_login(admin, correct):\n";
    std::cout << do_login("admin", "StrongPassw0rd!X") << "\n\n";

    std::cout << "TEST3 do_login(admin, wrong password):\n";
    std::cout << do_login("admin", "WrongPass123!") << "\n\n";

    std::cout << "TEST4 do_login(notadmin, correct password):\n";
    std::cout << do_login("notadmin", "StrongPassw0rd!X") << "\n\n";

    std::cout << "TEST5 do_login(invalid user, short password):\n";
    std::cout << do_login("bad user", "shortPwd!") << "\n";

    return 0;
}