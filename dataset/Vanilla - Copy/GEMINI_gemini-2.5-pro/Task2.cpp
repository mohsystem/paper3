#include <iostream>
#include <string>
#include <vector>
#include <sstream>

std::string createPhoneNumber(const int numbers[10]) {
    std::stringstream ss;
    ss << "(" << numbers[0] << numbers[1] << numbers[2] << ") "
       << numbers[3] << numbers[4] << numbers[5] << "-"
       << numbers[6] << numbers[7] << numbers[8] << numbers[9];
    return ss.str();
}

int main() {
    // Test cases
    int test_cases[5][10] = {
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {8, 0, 0, 5, 5, 5, 1, 2, 3, 4},
        {0, 9, 8, 7, 6, 5, 4, 3, 2, 1},
        {5, 1, 9, 5, 5, 5, 4, 4, 6, 8}
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << createPhoneNumber(test_cases[i]) << std::endl;
    }

    return 0;
}