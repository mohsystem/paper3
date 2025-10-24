#include <iostream>
#include <vector>
#include <stdexcept>

long long find_outlier(const std::vector<long long>& arr) {
    if (arr.size() < 3) {
        throw std::invalid_argument("Input array must have length >= 3");
    }
    int oddCount = 0;
    for (size_t i = 0; i < 3 && i < arr.size(); ++i) {
        if ((arr[i] & 1LL) != 0) ++oddCount;
    }
    bool majorityOdd = oddCount >= 2;
    for (long long v : arr) {
        bool vOdd = (v & 1LL) != 0;
        if (vOdd != majorityOdd) return v;
    }
    throw std::invalid_argument("No outlier found");
}

int main() {
    std::vector<std::vector<long long>> tests{
        {2, 4, 0, 100, 4, 11, 2602, 36},
        {160, 3, 1719, 19, 11, 13, -21},
        {3, 5, 7, 9, 2, 11, 13},
        {2, 4, 6, 8, 10, 3, 12},
        {-2, -4, -6, -8, -11, -12}
    };
    for (const auto& t : tests) {
        try {
            std::cout << find_outlier(t) << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error\n";
        }
    }
    return 0;
}