#include <iostream>
#include <unordered_set>
#include <string>
#include <vector>

std::string unsubscribe(const std::string& email, std::unordered_set<std::string>& database) {
    // Simulate DB access using the provided unordered_set
    if (database.find(email) != database.end()) {
        database.erase(email);
        return "Unsubscribed: " + email;
    } else {
        return "Email not found: " + email;
    }
}

int main() {
    std::unordered_set<std::string> database = {
        "alice@example.com",
        "bob@example.com",
        "carol@example.com"
    };

    std::vector<std::string> tests = {
        "alice@example.com",   // present -> removed
        "dave@example.com",    // not present
        "carol@example.com",   // present -> removed
        "eve@example.com",     // not present
        "bob@example.com"      // present -> removed
    };

    for (const auto& email : tests) {
        std::cout << unsubscribe(email, database) << "\n";
    }

    std::cout << "Remaining in database: ";
    bool first = true;
    for (const auto& e : database) {
        if (!first) std::cout << ", ";
        std::cout << e;
        first = false;
    }
    std::cout << "\n";

    return 0;
}