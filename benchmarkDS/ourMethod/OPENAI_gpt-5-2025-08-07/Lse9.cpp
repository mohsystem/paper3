#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <cstdlib>

std::pair<bool, std::string> arg_at_index(const std::vector<std::string>& argv, long index) {
    if (index < 0 || static_cast<size_t>(index) >= argv.size()) {
        return {false, std::string()};
    }
    return {true, argv[static_cast<size_t>(index)]};
}

static void run_tests() {
    std::vector<std::string> sample = {"prog", "alpha", "beta", "gamma", "delta"};
    long tests[5] = {0, 1, 3, 4, 10}; // 5 test cases
    for (long idx : tests) {
        auto res = arg_at_index(sample, idx);
        if (res.first) {
            std::cout << res.second << "\n";
        } else {
            std::cout << "ERROR\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        // Parse index from argv[1]
        char* endptr = nullptr;
        errno = 0;
        long idx = std::strtol(argv[1], &endptr, 10);
        if (errno == ERANGE || endptr == argv[1] || (endptr && *endptr != '\0') || idx < 0) {
            std::cout << "ERROR\n";
            return 0;
        }

        // Build vector of strings from actual argv (includes program name at index 0)
        std::vector<std::string> argsVec;
        argsVec.reserve(static_cast<size_t>(argc));
        for (int i = 0; i < argc; ++i) {
            argsVec.emplace_back(argv[i] ? argv[i] : "");
        }

        auto res = arg_at_index(argsVec, idx);
        if (res.first) {
            std::cout << res.second << "\n";
        } else {
            std::cout << "ERROR\n";
        }
    } else {
        // No args provided: run 5 test cases
        run_tests();
    }
    return 0;
}