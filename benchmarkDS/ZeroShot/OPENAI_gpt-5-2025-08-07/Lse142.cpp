#include <iostream>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <regex>
#include <cctype>

static std::unordered_set<std::string> DB;

static const size_t MAX_EMAIL_LENGTH = 254;
static const size_t MAX_LOCAL_LENGTH = 64;

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c));});
    return out;
}

static std::string normalize(const std::string& in) {
    std::string t = trim(in);
    if (t.empty() || t.size() > MAX_EMAIL_LENGTH) return std::string();
    return toLower(t);
}

static bool isValidEmail(const std::string& email) {
    if (email.empty() || email.size() > MAX_EMAIL_LENGTH) return false;
    static const std::regex re("^[A-Z0-9._%+-]{1,64}@[A-Z0-9.-]{1,253}\\.[A-Z]{2,63}$", std::regex::icase);
    if (!std::regex_match(email, re)) return false;

    auto atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0) return false;
    std::string local = email.substr(0, atPos);
    if (local.empty() || local.size() > MAX_LOCAL_LENGTH) return false;
    std::string domain = email.substr(atPos + 1);
    if (domain.empty() || domain.front() == '-' || domain.back() == '-') return false;
    if (domain.find("..") != std::string::npos) return false;
    size_t pos = 0;
    while (pos < domain.size()) {
        size_t dot = domain.find('.', pos);
        std::string label = domain.substr(pos, dot == std::string::npos ? std::string::npos : dot - pos);
        if (label.empty() || label.front() == '-' || label.back() == '-') return false;
        if (dot == std::string::npos) break;
        pos = dot + 1;
    }
    return true;
}

std::string unsubscribe(const std::string& emailInput) {
    try {
        std::string normalized = normalize(emailInput);
        if (normalized.empty() || !isValidEmail(normalized)) {
            return "Invalid email address.";
        }
        auto it = DB.find(normalized);
        if (it != DB.end()) {
            DB.erase(it);
            return "You have been unsubscribed.";
        } else {
            return "Email not found in database.";
        }
    } catch (...) {
        return "An error occurred processing your request.";
    }
}

static void seedDb() {
    DB.clear();
    DB.insert("a@example.com");
    DB.insert("b@example.org");
    DB.insert("c+news@domain.co");
    DB.insert("user@sub.domain.com");
    DB.insert("test@example.com");
}

int main() {
    seedDb();
    const std::string tests[5] = {
        "a@example.com",
        "  TEST@example.com  ",
        "notfound@nowhere.net",
        "invalid-email",
        "C+News@Domain.Co"
    };
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " -> " << unsubscribe(t) << "\n";
    }
    return 0;
}