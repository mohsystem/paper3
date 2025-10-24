// Chain-of-Through process in code generation:
// 1) Problem understanding:
//    Provide a function to validate IP addresses (IPv4/IPv6) and return "IPv4", "IPv6", or "Invalid".
// 2) Security requirements:
//    - Avoid regex pitfalls; implement strict parsers.
//    - No undefined behavior; careful index handling.
// 3) Secure coding generation:
//    - Manual parsing, bounds checks, validate char sets and segment counts.
// 4) Code review:
//    - Handle whitespace, leading zeros in IPv4, '::' rules in IPv6.
// 5) Secure code output:
//    - Final functions and main tests.

#include <bits/stdc++.h>
using namespace std;

static inline bool isHex(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static inline string trim(const string& s) {
    size_t i = 0, j = s.size();
    while (i < j && isspace(static_cast<unsigned char>(s[i]))) i++;
    while (j > i && isspace(static_cast<unsigned char>(s[j-1]))) j--;
    return s.substr(i, j - i);
}

static bool isValidIPv4(const string& s) {
    int n = (int)s.size();
    if (n == 0) return false;
    int segments = 0;
    int numDigits = 0;
    int value = 0;
    bool firstDigitZero = false;

    for (int idx = 0; idx < n; ++idx) {
        char c = s[idx];
        if (c == '.') {
            if (numDigits == 0) return false;
            segments++;
            if (segments > 3) return false;
            if (numDigits > 1 && firstDigitZero) return false;
            if (value > 255) return false;
            numDigits = 0;
            value = 0;
            firstDigitZero = false;
        } else if (c >= '0' && c <= '9') {
            if (numDigits == 0) firstDigitZero = (c == '0');
            if (numDigits >= 3) return false;
            value = value * 10 + (c - '0');
            numDigits++;
            if (value > 255) return false;
        } else {
            return false;
        }
    }
    if (numDigits == 0) return false;
    if (segments != 3) return false;
    if (numDigits > 1 && firstDigitZero) return false;
    if (value > 255) return false;
    return true;
}

static int parseIPv6Segment(const string& seg) {
    int n = (int)seg.size();
    if (n == 0) return 0;
    int i = 0;
    int count = 0;
    while (i < n) {
        int groupLen = 0;
        while (i < n && isHex(seg[i]) && groupLen < 4) {
            i++; groupLen++;
        }
        if (groupLen == 0) return -1;
        if (i < n && isHex(seg[i])) return -1; // group too long (>4)
        count++;
        if (i == n) break;
        if (seg[i] != ':') return -1;
        i++;
        if (i == n) return -1; // trailing colon
    }
    return count;
}

static bool isValidIPv6(const string& s) {
    size_t idx = s.find("::");
    if (idx != string::npos) {
        if ( s.find("::", idx + 1) != string::npos ) return false;
        string left = s.substr(0, idx);
        string right = s.substr(idx + 2);
        int lc = parseIPv6Segment(left);
        if (lc < 0) return false;
        int rc = parseIPv6Segment(right);
        if (rc < 0) return false;
        int total = lc + rc;
        if (total > 7) return false;
        int missing = 8 - total;
        if (missing < 1) return false;
        return true;
    } else {
        int count = parseIPv6Segment(s);
        return count == 8;
    }
}

string validateIPAddress(const string& ip) {
    string s = ip;
    // handle null equivalent not applicable in C++ but keep robust
    string t = trim(s);
    if (t.empty()) return "Invalid";
    if (isValidIPv4(t)) return "IPv4";
    if (isValidIPv6(t)) return "IPv6";
    return "Invalid";
}

int main() {
    vector<string> tests = {
        "172.16.254.1",
        "2001:0db8:85a3:0000:0000:8A2E:0370:7334",
        "256.256.256.256",
        "2001:db8::8a2e:370:7334",
        "01.1.1.1"
    };
    for (auto& t : tests) {
        cout << "Input: " << t << " -> " << validateIPAddress(t) << "\n";
    }
    return 0;
}