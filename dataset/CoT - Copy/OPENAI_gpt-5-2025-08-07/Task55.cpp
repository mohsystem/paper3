// Chain-of-Through secure implementation for email validation in C++

#include <iostream>
#include <string>

static bool isAlnumASCII(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}

static bool isAlphaASCII(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

static bool validateLocal(const std::string& local) {
    if (local.empty() || local.size() > 64) return false;
    if (local.front() == '.' || local.back() == '.') return false;
    bool prevDot = false;
    for (char c : local) {
        if (!(isAlnumASCII(c) || c == '.' || c == '_' || c == '+' || c == '-')) return false;
        if (c == '.') {
            if (prevDot) return false;
            prevDot = true;
        } else {
            prevDot = false;
        }
    }
    return true;
}

static bool validateDomain(const std::string& domain) {
    if (domain.empty() || domain.size() > 253) return false;
    size_t start = 0;
    size_t labelCount = 0;
    size_t lastLabelStart = 0, lastLabelLen = 0;

    while (true) {
        size_t dot = domain.find('.', start);
        size_t end = (dot == std::string::npos) ? domain.size() : dot;
        size_t len = (end >= start) ? (end - start) : 0;

        if (len == 0 || len > 63) return false;
        char first = domain[start];
        char last = domain[end - 1];
        if (!isAlnumASCII(first) || !isAlnumASCII(last)) return false;

        for (size_t i = start; i < end; ++i) {
            char c = domain[i];
            if (!(isAlnumASCII(c) || c == '-')) return false;
        }

        labelCount++;
        lastLabelStart = start;
        lastLabelLen = len;

        if (dot == std::string::npos) break;
        start = dot + 1;
    }

    if (labelCount < 2) return false;
    // TLD checks
    if (lastLabelLen < 2 || lastLabelLen > 63) return false;
    for (size_t i = 0; i < lastLabelLen; ++i) {
        char c = domain[lastLabelStart + i];
        if (!isAlphaASCII(c)) return false;
    }
    return true;
}

bool validateEmail(const std::string& email) {
    if (email.empty() || email.size() > 254) return false;
    for (char c : email) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc <= 32 || uc >= 127) return false; // reject control, space, non-ASCII
    }

    size_t atFirst = email.find('@');
    size_t atLast = email.rfind('@');
    if (atFirst == std::string::npos || atFirst != atLast || atFirst == 0 || atFirst == email.size() - 1)
        return false;

    std::string local = email.substr(0, atFirst);
    std::string domain = email.substr(atFirst + 1);
    return validateLocal(local) && validateDomain(domain);
}

static void runTest(const std::string& s) {
    std::cout << s << " -> " << (validateEmail(s) ? "true" : "false") << "\n";
}

int main() {
    // 5 test cases
    runTest("user.name+tag-123@example-domain.com"); // valid
    runTest("a_b@example.co");                       // valid
    runTest("user..dot@example.com");                // invalid
    runTest("user@example");                         // invalid
    runTest(".startingdot@example.com");             // invalid
    return 0;
}