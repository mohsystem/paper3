#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <vector>

static std::unordered_set<std::string> USER_DB;
static std::unordered_set<std::string> MAILING_LIST;

static std::string trimLower(const std::string& in) {
    size_t start = 0;
    while (start < in.size() && std::isspace(static_cast<unsigned char>(in[start]))) start++;
    size_t end = in.size();
    while (end > start && std::isspace(static_cast<unsigned char>(in[end - 1]))) end--;
    std::string s = in.substr(start, end - start);
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

static bool isAllowedChar(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
           c == '.' || c == '_' || c == '%' || c == '+' || c == '-' || c == '@';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z');
}

static bool isAlnum(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

static bool split(const std::string& s, char delim, std::vector<std::string>& out) {
    out.clear();
    size_t pos = 0;
    while (pos <= s.size()) {
        size_t next = s.find(delim, pos);
        if (next == std::string::npos) next = s.size();
        out.emplace_back(s.substr(pos, next - pos));
        pos = next + 1;
    }
    return true;
}

static bool sanitizeEmail(const std::string& input, std::string& out) {
    out = trimLower(input);
    if (out.size() < 3 || out.size() > 254) return false;
    for (char c : out) {
        if (!isAllowedChar(c)) return false;
    }
    size_t at = out.find('@');
    if (at == std::string::npos || at == 0 || at == out.size() - 1) return false;
    if (out.find('@', at + 1) != std::string::npos) return false;
    if (out.find("..") != std::string::npos) return false;

    std::string local = out.substr(0, at);
    std::string domain = out.substr(at + 1);
    if (local.empty() || domain.empty()) return false;

    size_t lastDot = domain.rfind('.');
    if (lastDot == std::string::npos || lastDot == 0 || lastDot >= domain.size() - 2) return false;

    std::string tld = domain.substr(lastDot + 1);
    if (tld.size() < 2 || tld.size() > 63) return false;
    for (char c : tld) if (!isAlpha(c)) return false;

    std::vector<std::string> labels;
    split(domain, '.', labels);
    for (const auto& label : labels) {
        if (label.empty()) return false;
        if (label.front() == '-' || label.back() == '-') return false;
        for (char c : label) {
            if (!(isAlnum(c) || c == '-')) return false;
        }
    }
    return true;
}

std::string unsubscribe_get(const std::string& emailParam) {
    std::string email;
    if (!sanitizeEmail(emailParam, email)) {
        return "Invalid email parameter.";
    }
    auto it = USER_DB.find(email);
    if (it != USER_DB.end()) {
        USER_DB.erase(it);
        MAILING_LIST.erase(email);
        return "You have been unsubscribed.";
    } else {
        return "The provided email is not subscribed.";
    }
}

static void seed() {
    USER_DB.clear();
    MAILING_LIST.clear();
    const char* seeds[] = {
        "alice@example.com",
        "bob@example.org",
        "carol.test+news@sub.domain.com"
    };
    for (const char* e : seeds) {
        std::string s;
        if (sanitizeEmail(e, s)) {
            USER_DB.insert(s);
            MAILING_LIST.insert(s);
        }
    }
}

int main() {
    seed();
    const std::string tests[5] = {
        "alice@example.com",
        " ALICE@EXAMPLE.COM ",
        "carol.test+news@sub.domain.com",
        "dave@example.com",
        "bad@@example..com"
    };
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " -> " << unsubscribe_get(t) << "\n";
    }
    return 0;
}