#include <iostream>
#include <string>
#include <charconv>
#include <optional>
#include <limits>
#include <cctype>

static std::optional<long long> addThousand(long long x) {
    const long long K = 1000LL;
    if (x > std::numeric_limits<long long>::max() - K) return std::nullopt;
    if (x < std::numeric_limits<long long>::min() + K) return std::nullopt;
    return x + K;
}

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string in = trim(argv[1] ? std::string(argv[1]) : std::string());
        long long value = 0;
        auto first = in.data();
        auto last = in.data() + in.size();
        std::from_chars_result res = std::from_chars(first, last, value, 10);
        if (res.ec != std::errc() || res.ptr != last) {
            std::cerr << "Error: Input must be a valid integer.\n";
            return 1;
        }
        auto out = addThousand(value);
        if (!out.has_value()) {
            std::cerr << "Error: Integer overflow.\n";
            return 1;
        }
        std::cout << out.value() << "\n";
    } else {
        // 5 test cases
        long long tests[5] = {
            0LL,
            1LL,
            -1000LL,
            std::numeric_limits<long long>::max() - 1000LL,
            std::numeric_limits<long long>::max()
        };
        for (long long t : tests) {
            auto r = addThousand(t);
            if (r.has_value()) {
                std::cout << "addThousand(" << t << ") = " << r.value() << "\n";
            } else {
                std::cout << "addThousand(" << t << ") = error: overflow\n";
            }
        }
    }
    return 0;
}