#include <iostream>
#include <unordered_map>
#include <string>

class Lse35 {
public:
    static std::unordered_map<std::string, std::string> DB;

    // Returns redirect path based on login success: "/home" or "/login"
    static std::string login(const std::string& username, const std::string& password) {
        auto it = DB.find(username);
        if (it != DB.end() && it->second == password) return "/home";
        return "/login";
    }
};

std::unordered_map<std::string, std::string> Lse35::DB = {
    {"alice", "secret"},
    {"bob", "hunter2"},
    {"eve", "password123"}
};

int main() {
    std::pair<std::string, std::string> tests[5] = {
        {"alice", "secret"},
        {"bob", "hunter2"},
        {"alice", "wrong"},
        {"unknown", "nopass"},
        {"eve", "password123"}
    };

    for (const auto& t : tests) {
        std::cout << "login(" << t.first << ", " << t.second << ") -> "
                  << Lse35::login(t.first, t.second) << "\n";
    }
    return 0;
}