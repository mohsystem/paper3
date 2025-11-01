#include <iostream>
#include <string>

bool auth_mod(const std::string& username, const std::string& password, const std::string& session) {
    bool isMod = username.rfind("mod_", 0) == 0; // starts with "mod_"
    return isMod && password == "secret" && session == "valid";
}

bool check_mod(const std::string& username, const std::string& password, const std::string& session) {
    return auth_mod(username, password, session);
}

int main() {
    std::pair<std::string, std::pair<std::string, std::string>> tests[] = {
        {"mod_alice", {"secret", "valid"}},
        {"user_bob", {"secret", "valid"}},
        {"mod_carol", {"wrong", "valid"}},
        {"mod_dan", {"secret", "expired"}},
        {"", {"", ""}}
    };
    for (int i = 0; i < 5; ++i) {
        bool res = check_mod(tests[i].first, tests[i].second.first, tests[i].second.second);
        std::cout << "Test " << (i + 1) << ": " << (res ? "true" : "false") << "\n";
    }
    return 0;
}