#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>

class Lse37 {
private:
    static std::unordered_map<std::string, std::string>& db() {
        static std::unordered_map<std::string, std::string> instance = {
            {"alice", "StrongPass!42"},
            {"bob", "s3cr3t"},
            {"charlie", "passw0rd"},
            {"diana", "Qwerty!234"}
        };
        return instance;
    }

    static bool constant_time_equals(const std::string& a, const std::string& b) {
        size_t maxlen = std::max(a.size(), b.size());
        unsigned int diff = 0;
        for (size_t i = 0; i < maxlen; ++i) {
            unsigned char ca = i < a.size() ? static_cast<unsigned char>(a[i]) : 0;
            unsigned char cb = i < b.size() ? static_cast<unsigned char>(b[i]) : 0;
            diff |= (ca ^ cb);
        }
        diff |= (a.size() == b.size()) ? 0u : 1u;
        return diff == 0;
    }

public:
    static std::string handleLogin(const std::string& username, const std::string& password) {
        try {
            const size_t MAX_USERNAME_LEN = 64;
            const size_t MAX_PASSWORD_LEN = 128;
            const std::string DUMMY_SECRET = "dummy_password_for_timing";

            if (username.empty() || username.size() > MAX_USERNAME_LEN || password.size() > MAX_PASSWORD_LEN) {
                return "cant_login";
            }
            if (username == "__simulate_error__") {
                throw std::runtime_error("Simulated internal error");
            }

            auto it = db().find(username);
            if (it == db().end()) {
                // Mitigate user enumeration
                (void)constant_time_equals(password, DUMMY_SECRET);
                return "cant_login";
            }
            bool ok = constant_time_equals(password, it->second);
            return ok ? "success" : "cant_login";
        } catch (...) {
            return "internal_server_error";
        }
    }
};

int main() {
    // 5 test cases
    std::cout << Lse37::handleLogin("alice", "StrongPass!42") << "\n"; // success
    std::cout << Lse37::handleLogin("alice", "wrong") << "\n";         // cant_login
    std::cout << Lse37::handleLogin("unknown", "whatever") << "\n";    // cant_login
    std::cout << Lse37::handleLogin("", "somepass") << "\n";           // cant_login
    std::cout << Lse37::handleLogin("__simulate_error__", "x") << "\n";// internal_server_error
    return 0;
}