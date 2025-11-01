#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>

std::string arg_at_index(long long index, const std::vector<std::string>& argv_including_program_name) {
    if (index < 0 || static_cast<size_t>(index) >= argv_including_program_name.size()) {
        return std::string();
    }
    return argv_including_program_name[static_cast<size_t>(index)];
}

static bool parse_non_negative_ll(const char* s, long long& out) {
    if (s == nullptr) return false;
    errno = 0;
    char* endptr = nullptr;
    long long val = std::strtoll(s, &endptr, 10);
    if (errno != 0 || endptr == s || *endptr != '\0') return false;
    if (val < 0) return false;
    out = val;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        long long idx = -1;
        if (!parse_non_negative_ll(argv[1], idx)) {
            std::cerr << "Error: index must be a non-negative integer.\n";
            return 1;
        }
        if (idx < 0 || idx >= argc) {
            std::cerr << "Error: index out of range.\n";
            return 1;
        }
        std::cout << argv[idx] << "\n";
        return 0;
    }

    // Test cases (run when no CLI args are provided)
    {
        std::vector<std::string> v1 = {"prog", "1", "alpha", "beta", "gamma"};
        std::cout << arg_at_index(0, v1) << "\n";
        std::cout << arg_at_index(1, v1) << "\n";
        std::cout << arg_at_index(2, v1) << "\n";
        std::vector<std::string> v2 = {"prog", "3", "foo", "bar", "baz"};
        std::cout << arg_at_index(3, v2) << "\n";
        std::vector<std::string> v3 = {"prog", "0"};
        std::cout << arg_at_index(0, v3) << "\n";
    }
    return 0;
}