#include <iostream>
#include <string>
#include <vector>
#include <utility>

class Lse39 {
public:
    static std::string login(const std::string& username, const std::string& password) {
        if (username == "admin" && password == "password123") {
            return "Redirecting to session page";
        }
        return "Incorrect username or password";
    }
};

int main() {
    std::vector<std::pair<std::string, std::string>> tests = {
        {"admin", "password123"},
        {"admin", "wrong"},
        {"user", "password123"},
        {"", "password123"},
        {"Admin", "password123"}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << ": "
                  << Lse39::login(tests[i].first, tests[i].second) << std::endl;
    }
    return 0;
}