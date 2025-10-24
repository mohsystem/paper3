#include <iostream>
#include <string>
#include <vector>
#include <limits>

namespace SafeAccum {
    static inline bool isAsciiLetter(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static inline char toUpperAscii(char c) {
        if (c >= 'a' && c <= 'z') return static_cast<char>(c - 'a' + 'A');
        return c;
    }

    static inline char toLowerAscii(char c) {
        if (c >= 'A' && c <= 'Z') return static_cast<char>(c - 'A' + 'a');
        return c;
    }

    std::string accum(const std::string& s) {
        // Fail closed: validate input length and characters
        const size_t MAX_LEN = 50000u; // Conservative upper bound to avoid pathological allocations
        const size_t n = s.size();
        if (n > MAX_LEN) {
            return std::string(); // empty string indicates invalid input or too large
        }
        for (char c : s) {
            if (!isAsciiLetter(c)) {
                return std::string(); // invalid character found
            }
        }

        // Calculate required size: sum_{i=0}^{n-1} (1 + i) letters + (n - 1) dashes
        // letters_total = n(n+1)/2
        unsigned long long letters_total_ull = (static_cast<unsigned long long>(n) * (static_cast<unsigned long long>(n) + 1ull)) / 2ull;
        unsigned long long dashes_ull = (n > 0) ? (static_cast<unsigned long long>(n) - 1ull) : 0ull;
        unsigned long long total_ull = letters_total_ull + dashes_ull;

        if (total_ull > static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
            return std::string(); // size overflow, fail closed
        }
        const size_t total_size = static_cast<size_t>(total_ull);

        std::string out;
        out.reserve(total_size);

        for (size_t i = 0; i < n; ++i) {
            char c = s[i];
            if (i != 0) out.push_back('-');
            out.push_back(toUpperAscii(c));
            for (size_t j = 0; j < i; ++j) {
                out.push_back(toLowerAscii(c));
            }
        }

        return out;
    }
}

int main() {
    using SafeAccum::accum;

    std::vector<std::string> tests = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "",
        "ZpG1" // invalid input (contains '1'), should produce empty string
    };

    for (const auto& t : tests) {
        std::string res = accum(t);
        std::cout << "accum(\"" << t << "\") -> \"" << res << "\"\n";
    }

    return 0;
}