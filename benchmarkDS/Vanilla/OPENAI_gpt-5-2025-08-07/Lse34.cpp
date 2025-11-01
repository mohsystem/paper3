#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

std::string loginRedirect(const std::string& username, const std::string& password,
                          const std::unordered_map<std::string, std::string>& database) {
    auto it = database.find(username);
    if (it != database.end() && it->second == password) {
        return "/dashboard";
    }
    return "/login";
}

int main() {
    std::unordered_map<std::string, std::string> db{
        {"alice", "pass123"},
        {"bob", "secret"},
        {"carol", "hunter2"}
    };

    std::vector<std::pair<std::string, std::string>> tests = {
        {"alice", "pass123"},
        {"alice", "wrong"},
        {"unknown", "pass123"},
        {"bob", "secret"},
        {"carol", "HUNTER2"}
    };

    int i = 1;
    for (const auto& t : tests) {
        std::string result = loginRedirect(t.first, t.second, db);
        std::cout << "Test " << i++ << " (" << t.first << "): " << result << "\n";
    }

    return 0;
}