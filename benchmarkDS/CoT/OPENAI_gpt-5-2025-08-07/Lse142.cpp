#include <iostream>
#include <unordered_set>
#include <string>
#include <vector>

class Lse142 {
private:
    std::unordered_set<std::string> database;

public:
    Lse142(const std::vector<std::string>& initialEmails) {
        for (const auto& e : initialEmails) database.insert(e);
    }

    std::string unsubscribe(const std::string& email) {
        auto it = database.find(email);
        if (it != database.end()) {
            database.erase(it);
            return "Email removed from database.";
        } else {
            return "Email not found in database.";
        }
    }
};

int main() {
    Lse142 service({ "alice@example.com", "bob@example.com", "carol@example.com" });

    std::vector<std::string> tests = {
        "alice@example.com",
        "dave@example.com",
        "bob@example.com",
        "alice@example.com",
        "carol@example.com"
    };

    for (const auto& email : tests) {
        std::cout << email << " -> " << service.unsubscribe(email) << std::endl;
    }

    return 0;
}