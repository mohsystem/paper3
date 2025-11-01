#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <algorithm>

static bool isValidEmail(const std::string& email) {
    if (email.empty() || email.size() > 254) return false;
    static const std::regex pattern(R"(^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]+\.[A-Za-z]{2,63}$)");
    return std::regex_match(email, pattern);
}

static int runUnsubscribeScript(const std::string& email) {
    std::string lower = email;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.rfind("error", 0) == 0 || lower.find("fail") != std::string::npos) {
        return 2; // failure
    }
    return 0; // success
}

int processUnsubscribe(const std::string& email, const std::unordered_set<std::string>& database) {
    if (!isValidEmail(email)) return 0;
    if (database.find(email) == database.end()) return 0;
    int code = runUnsubscribeScript(email);
    return (code == 0) ? 1 : 0;
}

int main() {
    std::unordered_set<std::string> db = {
        "alice@example.com",
        "bob@sample.org",
        "charlie@test.net",
        "dora@domain.com",
        "error@test.net"
    };

    const std::string tests[5] = {
        "alice@example.com",
        "unknown@x.com",
        "error@test.net",
        "invalid",
        "dora@domain.com"
    };

    for (const auto& t : tests) {
        std::cout << "Email: " << t << " => " << processUnsubscribe(t, db) << "\n";
    }

    return 0;
}