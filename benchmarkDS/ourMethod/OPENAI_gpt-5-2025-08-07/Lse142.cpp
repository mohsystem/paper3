#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <algorithm>
#include <cctype>

class Database {
public:
    static Database& connect() {
        static Database instance;
        return instance;
    }

    bool remove(const std::string& emailLower) {
        auto it = emails.find(emailLower);
        if (it != emails.end()) {
            emails.erase(it);
            return true;
        }
        return false;
    }

private:
    std::unordered_set<std::string> emails;

    Database() {
        emails.insert("user@example.com");
        emails.insert("alice@test.com");
        emails.insert("bob@example.org");
        emails.insert("newsletter@company.com");
        emails.insert("sales@shop.co");
    }
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
};

static const size_t MAX_EMAIL_LEN = 254;

static bool is_ascii(const std::string& s) {
    for (unsigned char ch : s) {
        if (ch > 0x7F) return false;
    }
    return true;
}

static bool sanitizeEmail(const std::string& input, std::string& outLower) {
    std::string trimmed = input;
    // Trim leading/trailing spaces
    auto notSpace = [](int c){ return !std::isspace(c); };
    trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), notSpace));
    trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), notSpace).base(), trimmed.end());

    if (trimmed.empty() || trimmed.size() > MAX_EMAIL_LEN) return false;
    if (!is_ascii(trimmed)) return false;

    static const std::regex emailRe(R"(^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]+\.[A-Za-z]{2,63}$)");
    if (!std::regex_match(trimmed, emailRe)) return false;

    outLower.resize(trimmed.size());
    std::transform(trimmed.begin(), trimmed.end(), outLower.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return true;
}

std::string unsubscribe(const std::string& email) {
    std::string lower;
    if (!sanitizeEmail(email, lower)) {
        return "Invalid email address.";
    }
    try {
        Database& db = Database::connect();
        bool removed = db.remove(lower);
        return removed ? "Unsubscribed successfully." : "Email address not found in database.";
    } catch (...) {
        return "An error occurred.";
    }
}

int main() {
    const std::string tests[5] = {
        "user@example.com",    // should unsubscribe
        "ALICE@test.com",      // unsubscribe (case-insensitive)
        "nobody@nope.com",     // not found
        "bad@@domain..com",    // invalid
        "user@example.com"     // already unsubscribed -> not found
    };
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " -> " << unsubscribe(t) << "\n";
    }
    return 0;
}