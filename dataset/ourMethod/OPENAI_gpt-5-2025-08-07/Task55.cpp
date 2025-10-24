#include <iostream>
#include <string>
#include <vector>
#include <cctype>

static bool isAsciiLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool isAsciiDigit(char c) {
    return (c >= '0' && c <= '9');
}

static bool isLocalAllowed(char c) {
    // Allowed specials in dot-atom local part per RFC 5322 (subset)
    switch (c) {
        case '!': case '#': case '$': case '%': case '&':
        case '\'': case '*': case '+': case '/': case '=':
        case '?': case '^': case '_': case '`': case '{':
        case '|': case '}': case '~': case '-':
            return true;
        default:
            break;
    }
    return isAsciiLetter(c) || isAsciiDigit(c);
}

static bool validateLocalPart(const std::string& s) {
    if (s.empty() || s.size() > 64) return false;
    if (s.front() == '.' || s.back() == '.') return false;

    bool prevDot = false;
    for (char c : s) {
        if (static_cast<unsigned char>(c) < 33 || static_cast<unsigned char>(c) > 126) return false; // printable ASCII excl space
        if (c == '.') {
            if (prevDot) return false;
            prevDot = true;
        } else {
            prevDot = false;
            if (!isLocalAllowed(c)) return false;
        }
    }
    return true;
}

static bool validateDomain(const std::string& s) {
    if (s.empty() || s.size() > 253) return false;
    if (s.front() == '.' || s.back() == '.') return false;

    bool prevDot = false;
    bool hasDot = false;

    size_t labelLen = 0;
    bool labelAllAlpha = true; // track for current label (TLD must be alpha-only and len >= 2)
    bool labelStarted = false;
    char lastChar = '\0';

    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc < 33 || uc > 126) return false; // printable ASCII excl space

        if (c == '.') {
            if (prevDot) return false;
            if (!labelStarted || labelLen == 0) return false;
            if (lastChar == '-') return false; // label cannot end with hyphen
            if (labelLen > 63) return false;

            hasDot = true;
            // reset for next label
            prevDot = true;
            labelLen = 0;
            labelAllAlpha = true;
            labelStarted = false;
            lastChar = '.';
            continue;
        }

        prevDot = false;
        // label char validations
        if (!(isAsciiLetter(c) || isAsciiDigit(c) || c == '-')) return false;

        if (!labelStarted) {
            // first char of label cannot be hyphen
            if (c == '-') return false;
            labelStarted = true;
        }

        if (!isAsciiLetter(c)) {
            labelAllAlpha = false;
        }

        ++labelLen;
        lastChar = c;

        // Enforce upper bound as we go to avoid overflow
        if (labelLen > 63) return false;
    }

    // Final label checks
    if (!labelStarted || labelLen == 0) return false;
    if (lastChar == '-') return false;
    if (labelLen > 63) return false;
    if (!hasDot) return false; // require at least one dot in domain

    // TLD rule: final label must be all letters and length >= 2
    if (!(labelAllAlpha && labelLen >= 2)) return false;

    return true;
}

bool isValidEmail(const std::string& email) {
    if (email.empty() || email.size() > 254) return false;

    // Must contain exactly one '@' and not be first/last character
    size_t atPos = std::string::npos;
    size_t countAt = 0;
    for (size_t i = 0; i < email.size(); ++i) {
        if (email[i] == '@') {
            ++countAt;
            if (countAt == 1) atPos = i;
        }
    }
    if (countAt != 1 || atPos == 0 || atPos == email.size() - 1) return false;

    std::string local = email.substr(0, atPos);
    std::string domain = email.substr(atPos + 1);

    if (!validateLocalPart(local)) return false;
    if (!validateDomain(domain)) return false;

    return true;
}

int main() {
    std::vector<std::string> tests = {
        "alice.smith+tag@example.co.uk",    // valid
        "bad..dots@domain.com",             // invalid
        "user@-example.com",                // invalid
        "customer/department=shipping@example.com", // valid
        "user@example.c"                    // invalid (TLD too short)
    };

    for (const auto& e : tests) {
        bool ok = isValidEmail(e);
        std::cout << e << " -> " << (ok ? "valid" : "invalid") << "\n";
    }
    return 0;
}