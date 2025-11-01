#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>
#include <unordered_set>

static bool isValidEmail(const std::string& email) {
    if (email.empty()) return false;
    // trim
    auto l = email.find_first_not_of(" \t\r\n");
    auto r = email.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) return false;
    std::string s = email.substr(l, r - l + 1);
    if (s.size() < 3 || s.size() > 254) return false;

    auto atPos = s.find('@');
    if (atPos == std::string::npos || atPos == 0 || atPos != s.rfind('@') || atPos == s.size() - 1) return false;

    std::string local = s.substr(0, atPos);
    std::string domain = s.substr(atPos + 1);
    if (local.empty() || local.size() > 64) return false;

    if (local.front() == '.' || local.back() == '.') return false;
    bool prevDot = false;
    for (char ch : local) {
        bool ok = std::isalnum(static_cast<unsigned char>(ch)) || ch == '.' || ch == '_' || ch == '+' || ch == '-';
        if (!ok) return false;
        if (ch == '.') {
            if (prevDot) return false;
            prevDot = true;
        } else {
            prevDot = false;
        }
    }

    if (domain.find('.') == std::string::npos) return false;
    size_t start = 0;
    std::vector<std::string> labels;
    while (true) {
        auto dot = domain.find('.', start);
        if (dot == std::string::npos) {
            labels.emplace_back(domain.substr(start));
            break;
        }
        labels.emplace_back(domain.substr(start, dot - start));
        start = dot + 1;
    }
    if (labels.size() < 2) return false;
    for (const auto& label : labels) {
        if (label.empty() || label.size() > 63) return false;
        if (label.front() == '-' || label.back() == '-') return false;
        for (char ch : label) {
            if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '-')) return false;
        }
    }
    if (labels.back().size() < 2) return false;
    return true;
}

static std::string normalizeEmail(const std::string& email) {
    auto l = email.find_first_not_of(" \t\r\n");
    auto r = email.find_last_not_of(" \t\r\n");
    std::string s = (l == std::string::npos) ? std::string() : email.substr(l, r - l + 1);
    std::string out;
    out.reserve(s.size());
    for (char ch : s) out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    return out;
}

// Returns 1 if email exists and script returns 0, else 0
static int processUnsubscribe(const std::string& email,
                              const std::vector<std::string>& database,
                              const std::function<int(const std::string&)>& script) {
    if (!isValidEmail(email)) return 0;
    std::unordered_set<std::string> dbset;
    dbset.reserve(database.size() * 2 + 1);
    for (const auto& e : database) {
        if (isValidEmail(e)) dbset.insert(normalizeEmail(e));
    }
    std::string norm = normalizeEmail(email);
    if (dbset.find(norm) == dbset.end()) return 0;
    int rc;
    try {
        rc = script(norm);
    } catch (...) {
        return 0;
    }
    return rc == 0 ? 1 : 0;
}

// Example script: success for @example.com
static int exampleScript(const std::string& email) {
    const std::string suf = "@example.com";
    if (email.size() >= suf.size() && email.compare(email.size() - suf.size(), suf.size(), suf) == 0) {
        return 0;
    }
    return 5;
}

int main() {
    std::vector<std::string> db = {
        "alice@example.com",
        "bob@example.com",
        "carol@block.example",
        "dave@example.org",
        "eve@example.net"
    };

    std::vector<std::string> tests = {
        "alice@example.com",     // 1
        "carol@block.example",   // 0
        "frank@example.com",     // 0
        "bad@@example.com",      // 0
        "dave@example.org"       // 0
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        int res = processUnsubscribe(tests[i], db, exampleScript);
        std::cout << "Test " << (i + 1) << ": " << res << "\n";
    }
    return 0;
}