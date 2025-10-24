#include <iostream>
#include <string>
#include <cctype>

// Function to transform a string as per the requirements
std::string accum(const std::string& s) {
    // According to the problem description, the input string includes only letters from a..z and A..Z.
    // However, as a good practice, we validate the input.
    for (char c : s) {
        if (!isalpha(static_cast<unsigned char>(c))) {
            // Return an empty string to indicate invalid input.
            return "";
        }
    }

    if (s.empty()) {
        return "";
    }

    std::string result;
    size_t n = s.length();
    
    // Pre-calculate the total length of the resulting string to reserve memory,
    // which avoids multiple reallocations and improves performance.
    // The length of the i-th segment (0-indexed) is i+1.
    // Total length of all segments is sum of (i+1) for i=0 to n-1, which is n*(n+1)/2.
    // There are n-1 hyphens for n > 1.
    size_t total_len = (n * (n + 1)) / 2;
    if (n > 1) {
        total_len += n - 1;
    }
    result.reserve(total_len);

    for (size_t i = 0; i < n; ++i) {
        if (i > 0) {
            result += '-';
        }
        result += toupper(static_cast<unsigned char>(s[i]));
        for (size_t j = 0; j < i; ++j) {
            result += tolower(static_cast<unsigned char>(s[i]));
        }
    }
    return result;
}

int main() {
    // 5 test cases
    std::cout << "accum(\"abcd\") -> \"" << accum("abcd") << "\"\n";
    std::cout << "accum(\"RqaEzty\") -> \"" << accum("RqaEzty") << "\"\n";
    std::cout << "accum(\"cwAt\") -> \"" << accum("cwAt") << "\"\n";
    std::cout << "accum(\"Z\") -> \"" << accum("Z") << "\"\n";
    std::cout << "accum(\"\") -> \"" << accum("") << "\"\n";
    return 0;
}