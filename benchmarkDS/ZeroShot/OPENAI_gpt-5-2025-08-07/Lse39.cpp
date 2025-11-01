#include <iostream>
#include <cstring>
#include <string>

struct Cred {
    const char* username;
    const char* password; // Expected password (demo only)
};

static const Cred USER_STORE[] = {
    {"alice", "Str0ngP@ss!"},
    {"admin", "S3cure#Admin"}
};

static const char* DUMMY_SECRET = "Dummy#Secret1";

// Constant-time comparison
bool secure_equals(const char* a, size_t alen, const char* b, size_t blen) {
    size_t maxlen = (alen > blen) ? alen : blen;
    unsigned int diff = static_cast<unsigned int>(alen ^ blen);
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char x = (i < alen) ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char y = (i < blen) ? static_cast<unsigned char>(b[i]) : 0;
        diff |= (x ^ y);
    }
    return diff == 0u;
}

// Login function: accepts parameters and returns result
std::string login(const std::string& username, const std::string& password) {
    if (username.empty()) {
        // Still perform dummy compare to keep timing uniform
        secure_equals(password.c_str(), password.size(), DUMMY_SECRET, std::strlen(DUMMY_SECRET));
        return "Invalid username or password.";
    }

    const char* expected = nullptr;
    bool userExists = false;
    for (const auto& c : USER_STORE) {
        if (username == c.username) {
            expected = c.password;
            userExists = true;
            break;
        }
    }
    if (!userExists) {
        expected = DUMMY_SECRET;
    }

    bool ok = secure_equals(password.c_str(), password.size(), expected, std::strlen(expected));
    if (ok && userExists) {
        return "SESSION: " + username;
    }
    return "Invalid username or password.";
}

int main() {
    // 5 test cases
    std::cout << login("alice", "Str0ngP@ss!") << "\n";
    std::cout << login("alice", "wrong") << "\n";
    std::cout << login("admin", "S3cure#Admin") << "\n";
    std::cout << login("eve", "anything") << "\n";
    std::cout << login("", "S3cure#Admin") << "\n";
    return 0;
}