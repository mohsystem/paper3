#include <iostream>
#include <vector>
#include <unordered_map>
#include <utility>

std::pair<int, int> two_sum(const std::vector<int>& numbers, int target) {
    if (numbers.size() < 2) {
        return {-1, -1};
    }

    std::unordered_map<long long, int> indexByValue;
    indexByValue.reserve(numbers.size());

    for (size_t i = 0; i < numbers.size(); ++i) {
        long long val = static_cast<long long>(numbers[i]);
        long long comp = static_cast<long long>(target) - val;

        auto it = indexByValue.find(comp);
        if (it != indexByValue.end() && static_cast<int>(i) != it->second) {
            return {it->second, static_cast<int>(i)};
        }
        // Store earliest index for a value to ensure distinct indices
        if (indexByValue.find(val) == indexByValue.end()) {
            indexByValue[val] = static_cast<int>(i);
        }
    }
    return {-1, -1};
}

static void print_result(const std::pair<int,int>& res) {
    if (res.first >= 0 && res.second >= 0) {
        std::cout << "(" << res.first << ", " << res.second << ")";
    } else {
        std::cout << "(not found)";
    }
}

int main() {
    // Test 1
    {
        std::vector<int> a{1, 2, 3};
        int target = 4;
        auto res = two_sum(a, target);
        std::cout << "Test 1: ";
        print_result(res);
        std::cout << "\n";
    }
    // Test 2
    {
        std::vector<int> a{3, 2, 4};
        int target = 6;
        auto res = two_sum(a, target);
        std::cout << "Test 2: ";
        print_result(res);
        std::cout << "\n";
    }
    // Test 3
    {
        std::vector<int> a{2, 7, 11, 15};
        int target = 9;
        auto res = two_sum(a, target);
        std::cout << "Test 3: ";
        print_result(res);
        std::cout << "\n";
    }
    // Test 4 (duplicates and zero)
    {
        std::vector<int> a{0, 4, 3, 0};
        int target = 0;
        auto res = two_sum(a, target);
        std::cout << "Test 4: ";
        print_result(res);
        std::cout << "\n";
    }
    // Test 5 (negative numbers)
    {
        std::vector<int> a{-3, 4, 3, 90};
        int target = 0;
        auto res = two_sum(a, target);
        std::cout << "Test 5: ";
        print_result(res);
        std::cout << "\n";
    }

    return 0;
}