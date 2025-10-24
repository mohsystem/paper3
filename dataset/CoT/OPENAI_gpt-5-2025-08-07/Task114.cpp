// Chain-of-Through Secure Implementation
// 1) Problem: safe string operations.
// 2) Security: no raw buffers, bounds/overflow checks.
// 3) Secure coding: std::string, validated inputs.
// 4) Review: no dangerous casts/undefined behavior.
// 5) Output: robust functions + tests.

#include <iostream>
#include <string>
#include <limits>

std::string copyString(const std::string& s) {
    return std::string(s);
}

std::string concatStrings(const std::string& a, const std::string& b) {
    std::string out;
    out.reserve(a.size() + b.size());
    out.append(a);
    out.append(b);
    return out;
}

std::string reverseString(const std::string& s) {
    std::string r = s;
    std::reverse(r.begin(), r.end());
    return r;
}

std::string replaceSubstring(const std::string& s, const std::string& target, const std::string& replacement) {
    if (target.empty()) return copyString(s);
    std::string out;
    size_t pos = 0;
    size_t idx;
    // Guard reserve size to avoid overflow
    if (s.size() < (std::numeric_limits<size_t>::max)() / 2) {
        out.reserve(s.size());
    }
    while ((idx = s.find(target, pos)) != std::string::npos) {
        out.append(s, pos, idx - pos);
        out.append(replacement);
        pos = idx + target.size();
    }
    out.append(s, pos, std::string::npos);
    return out;
}

std::string safeSubstring(const std::string& s, long long start, long long length) {
    if (start < 0) start = 0;
    if (length < 0) length = 0;
    size_t n = s.size();
    size_t st = static_cast<size_t>(std::min<long long>(start, (long long)n));
    if (st >= n || length == 0) return "";
    unsigned long long endULL = (unsigned long long)st + (unsigned long long)length;
    size_t end = (size_t)std::min<unsigned long long>(endULL, n);
    if (end < st) return "";
    return s.substr(st, end - st);
}

size_t countOccurrences(const std::string& s, const std::string& sub) {
    if (sub.empty()) return 0;
    size_t count = 0;
    size_t pos = 0;
    while (true) {
        size_t idx = s.find(sub, pos);
        if (idx == std::string::npos) break;
        count++;
        pos = idx + sub.size();
    }
    return count;
}

std::string repeatString(const std::string& s, size_t times) {
    if (times == 0 || s.empty()) return "";
    // overflow guard
    if (s.size() > 0 && times > (std::numeric_limits<size_t>::max)() / s.size()) {
        throw std::length_error("Result too large");
    }
    std::string out;
    out.reserve(s.size() * times);
    for (size_t i = 0; i < times; ++i) out += s;
    return out;
}

std::string toUpper(const std::string& s) {
    std::string r = s;
    for (auto& ch : r) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    return r;
}

std::string toLower(const std::string& s) {
    std::string r = s;
    for (auto& ch : r) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return r;
}

std::string trimSafe(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    return s.substr(i, j - i);
}

int main() {
    std::pair<std::string, std::string> pairs[5] = {
        {"Hello", "World"},
        {"", "Test"},
        {"  Spaces  ", ""},
        {"abcabcabc", "xyz"},
        {"", ""} // representing "null" equivalent by empty string
    };
    std::pair<std::string, std::string> repls[5] = {
        {"l", "L"},
        {"Test", "Exam"},
        {" ", ""},
        {"abc", "A"},
        {"", ""} // empty target/replacement
    };

    for (int i = 0; i < 5; ++i) {
        const auto& a = pairs[i].first;
        const auto& b = pairs[i].second;
        const auto& tgt = repls[i].first;
        const auto& rep = repls[i].second;

        std::cout << "=== Test " << (i + 1) << " ===\n";
        std::cout << "copyString(a): " << copyString(a) << "\n";
        std::cout << "concatStrings(a,b): " << concatStrings(a, b) << "\n";
        std::cout << "reverseString(a): " << reverseString(a) << "\n";
        std::cout << "replaceSubstring(a,tgt,rep): " << replaceSubstring(a, tgt, rep) << "\n";
        std::cout << "safeSubstring(a,1,3): " << safeSubstring(a, 1, 3) << "\n";
        std::cout << "countOccurrences(a,tgt): " << countOccurrences(a, tgt) << "\n";
        std::cout << "repeatString(\"ab\",3): " << repeatString(std::string("ab"), 3) << "\n";
        std::cout << "toUpper(a): " << toUpper(a) << "\n";
        std::cout << "toLower(a): " << toLower(a) << "\n";
        std::cout << "trimSafe(a): " << trimSafe(a) << "\n";
    }
    return 0;
}