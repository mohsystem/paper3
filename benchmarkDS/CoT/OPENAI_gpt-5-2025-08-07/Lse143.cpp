#include <iostream>
#include <unordered_set>
#include <string>

static std::unordered_set<std::string> database;
static std::unordered_set<std::string> mailingList;

void initializeDB() {
    database.clear();
    mailingList.clear();
    database.insert("alice@example.com");
    database.insert("bob@example.com");
    database.insert("dana@example.com");
    mailingList = database;
}

std::string unsubscribe(const std::string& email) {
    if (email.empty()) return "Invalid email";
    auto it = database.find(email);
    if (it != database.end()) {
        database.erase(it);
        mailingList.erase(email);
        return "Unsubscribed: " + email;
    } else {
        return "User not subscribed: " + email;
    }
}

int main() {
    initializeDB();
    std::string tests[5] = {
        "alice@example.com",
        "bob@example.com",
        "charlie@example.com",
        "bob@example.com",
        "dana@example.com"
    };
    for (const auto& t : tests) {
        std::cout << unsubscribe(t) << std::endl;
    }
    return 0;
}