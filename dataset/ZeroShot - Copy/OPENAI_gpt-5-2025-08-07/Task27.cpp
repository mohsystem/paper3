#include <iostream>
#include <vector>
#include <stdexcept>

long long findOutlier(const std::vector<long long>& arr) {
    if (arr.size() < 3) {
        throw std::invalid_argument("Input array must have length >= 3");
    }
    int evenCount = 0;
    for (size_t i = 0; i < 3; ++i) {
        if ( (arr[i] & 1LL) == 0LL ) {
            ++evenCount;
        }
    }
    bool majorityEven = evenCount >= 2;
    for (const auto& v : arr) {
        bool isEven = (v & 1LL) == 0LL;
        if (isEven != majorityEven) {
            return v;
        }
    }
    throw std::runtime_error("No outlier found");
}

int main() {
    std::vector<std::vector<long long>> tests = {
        {2, 4, 0, 100, 4, 11, 2602, 36},
        {160, 3, 1719, 19, 11, 13, -21},
        {-2, -4, -6, -8, -10, -3, -12},
        {1, 3, 5, 7, 8, 9, 11, 13},
        {42, 6, 8, 10, 12, 14, 16, 7}
    };

    for (const auto& t : tests) {
        try {
            std::cout << findOutlier(t) << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}