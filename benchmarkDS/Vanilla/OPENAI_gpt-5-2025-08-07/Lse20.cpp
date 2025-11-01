#include <iostream>
#include <cstdlib>

static const int OFFSET = 1000;

int add_offset(int value) {
    return value + OFFSET;
}

void run_tests() {
    int tests[5] = {0, 1, -1, 42, 999};
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test " << tests[i] << " -> " << add_offset(tests[i]) << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* endptr = nullptr;
        long v = std::strtol(argv[1], &endptr, 10);
        if (endptr != argv[1] && *endptr == '\0') {
            std::cout << add_offset(static_cast<int>(v)) << std::endl;
        } else {
            std::cout << "Invalid integer input" << std::endl;
        }
    }
    run_tests();
    return 0;
}