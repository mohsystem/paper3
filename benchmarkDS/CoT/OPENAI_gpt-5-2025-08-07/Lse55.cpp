#include <iostream>
#include <unordered_map>
#include <string>
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstring>

class Lse55 {
    // Step 1: Problem understanding - Endpoints with sessions and RBAC.
    // Step 2: Security requirements - Secure tokens, constant-time comparison, session expiry, minimal leakage.
    // Step 3: Secure coding generation - Implement careful method checks and input validation.
    // Step 4: Code review - Avoid sensitive outputs, robust token handling.
    // Step 5: Secure code output - Final version.

private:
    struct Session {
        std::string user;
        std::string role;
        long long expiry;
    };

    static std::unordered_map<std::string, std::string>& userPasswords() {
        static std::unordered_map<std::string, std::string> map = {
            {"user", "userpass"},
            {"admin", "adminpass"}
        };
        return map;
    }

    static std::unordered_map<std::string, std::string>& userRoles() {
        static std::unordered_map<std::string, std::string> map = {
            {"user", "user"},
            {"admin", "admin"}
        };
        return map;
    }

    static std::unordered_map<std::string, Session>& sessions() {
        static std::unordered_map<std::string, Session> s;
        return s;
    }

    static constexpr const char* DUMMY_PASSWORD = "dummy_password_fallback_value";
    static constexpr long long SESSION_TTL_MS = 15LL * 60LL * 1000LL;

    static long long nowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    static bool constantTimeEquals(const std::string& a, const std::string& b) {
        const size_t len = (a.size() > b.size()) ? a.size() : b.size();
        unsigned char diff = (unsigned char)(a.size() ^ b.size());
        for (size_t i = 0; i < len; ++i) {
            unsigned char ac = i < a.size() ? (unsigned char)a[i] : 0;
            unsigned char bc = i < b.size() ? (unsigned char)b[i] : 0;
            diff |= (unsigned char)(ac ^ bc);
        }
        return diff == 0;
    }

    static bool getRandomBytes(std::vector<unsigned char>& out) {
        // Try /dev/urandom
        std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
        if (urandom.good()) {
            urandom.read(reinterpret_cast<char*>(out.data()), out.size());
            if (urandom.gcount() == (std::streamsize)out.size()) {
                return true;
            }
        }
        // Fallback to std::random_device (may not be CSPRNG on all platforms)
        std::random_device rd;
        for (size_t i = 0; i < out.size(); ++i) {
            out[i] = static_cast<unsigned char>(rd());
        }
        return true;
    }

    static std::string toHex(const std::vector<unsigned char>& data) {
        std::ostringstream oss;
        for (auto b : data) {
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        }
        return oss.str();
    }

    static std::string generateToken() {
        std::vector<unsigned char> buf(32);
        getRandomBytes(buf);
        return toHex(buf);
    }

    static const Session* getValidSession(const std::string& token) {
        if (token.empty()) return nullptr;
        auto& s = sessions();
        auto it = s.find(token);
        if (it == s.end()) return nullptr;
        if (nowMs() > it->second.expiry) {
            s.erase(it);
            return nullptr;
        }
        return &it->second;
    }

public:
    // login (GET)
    static std::string login(const std::string& method) {
        if (method != "GET") return "405 Method Not Allowed";
        return "Login Page: Please provide username and password via do_login (POST).";
    }

    // do_login (POST) -> returns token or empty string on failure
    static std::string do_login(const std::string& method, const std::string& username, const std::string& password) {
        if (method != "POST") return std::string();
        const auto& up = userPasswords();
        auto it = up.find(username);
        std::string stored = (it != up.end()) ? it->second : std::string(DUMMY_PASSWORD);
        bool ok = constantTimeEquals(password, stored) && (it != up.end());
        if (!ok) return std::string();
        const auto& roles = userRoles();
        auto rit = roles.find(username);
        std::string role = (rit != roles.end()) ? rit->second : "user";
        std::string token = generateToken();
        sessions()[token] = Session{username, role, nowMs() + SESSION_TTL_MS};
        return token;
    }

    // index (GET)
    static std::string index(const std::string& method, const std::string& sessionToken) {
        if (method != "GET") return "405 Method Not Allowed";
        auto s = getValidSession(sessionToken);
        if (!s) return "Welcome, guest. Please login.";
        return "Welcome, " + s->user + "!";
    }

    // user_page (GET)
    static std::string user_page(const std::string& method, const std::string& sessionToken) {
        if (method != "GET") return "405 Method Not Allowed";
        auto s = getValidSession(sessionToken);
        if (!s) return "401 Unauthorized";
        return "User Page for " + s->user;
    }

    // admin_page (GET)
    static std::string admin_page(const std::string& method, const std::string& sessionToken) {
        if (method != "GET") return "405 Method Not Allowed";
        auto s = getValidSession(sessionToken);
        if (!s) return "401 Unauthorized";
        if (s->role != "admin") return "403 Forbidden";
        return "Admin Page for " + s->user;
    }
};

int main() {
    // 5 test cases
    std::cout << "Test1: " << Lse55::login("GET") << "\n";
    std::string userToken = Lse55::do_login("POST", "user", "userpass");
    std::cout << "Test2: user login token: " << (!userToken.empty() ? "true" : "false") << "\n";
    std::cout << "Test3: user page with user token: " << Lse55::user_page("GET", userToken) << "\n";
    std::cout << "Test4: admin page with user token: " << Lse55::admin_page("GET", userToken) << "\n";
    std::string adminToken = Lse55::do_login("POST", "admin", "adminpass");
    std::cout << "Test5: admin page with admin token: " << Lse55::admin_page("GET", adminToken) << "\n";
    return 0;
}