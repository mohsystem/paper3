#include <iostream>
#include <unordered_map>
#include <string>
#include <cctype>

static std::unordered_map<std::string, std::string> DB = {
    {"alice", "Wonderland!42"},
    {"bob", "Secure*Pass9"},
    {"charlie", "S3cret#Key"}
};

static const std::string DUMMY_PASSWORD = "dummy_secret_password_123";

static bool isPrintableAscii(const std::string& s) {
    for (unsigned char ch : s) {
        if (ch < 32 || ch > 126) return false;
    }
    return true;
}

static bool constantTimeEquals(const std::string& a, const std::string& b) {
    size_t maxLen = a.size() > b.size() ? a.size() : b.size();
    unsigned int diff = static_cast<unsigned int>(a.size() ^ b.size());
    for (size_t i = 0; i < maxLen; ++i) {
        unsigned char ba = i < a.size() ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char bb = i < b.size() ? static_cast<unsigned char>(b[i]) : 0;
        diff |= (ba ^ bb);
    }
    return diff == 0;
}

std::string login(const std::string& username, const std::string& password) {
    try {
        if (username.empty() || password.empty()) {
            return "internal_server_error";
        }
        if (username.size() > 128 || password.size() > 128) {
            return "internal_server_error";
        }
        if (!isPrintableAscii(username) || !isPrintableAscii(password)) {
            return "internal_server_error";
        }

        auto it = DB.find(username);
        const std::string& compareAgainst = (it != DB.end()) ? it->second : DUMMY_PASSWORD;
        bool match = constantTimeEquals(password, compareAgainst);

        if (it != DB.end() && match) {
            return "success";
        } else {
            return "cant_login";
        }
    } catch (...) {
        return "internal_server_error";
    }
}

int main() {
    // 5 test cases
    std::cout << login("alice", "Wonderland!42") << "\n"; // success
    std::cout << login("bob", "Secure*Pass9") << "\n";    // success
    std::cout << login("alice", "NotIt") << "\n";         // cant_login
    std::cout << login("dave", "anything") << "\n";       // cant_login
    std::cout << login("", "x") << "\n";                  // internal_server_error
    return 0;
}