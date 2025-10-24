#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>

std::pair<int, int> two_sum(const std::vector<int>& numbers, int target) {
    if (numbers.size() < 2) {
        throw std::invalid_argument("Invalid input");
    }
    std::unordered_map<int, int> mp;
    mp.reserve(numbers.size());
    for (int i = 0; i < static_cast<int>(numbers.size()); ++i) {
        int num = numbers[i];
        int comp = target - num;
        auto it = mp.find(comp);
        if (it != mp.end()) {
            return {it->second, i};
        }
        // keep earliest index for duplicates
        if (mp.find(num) == mp.end()) {
            mp[num] = i;
        }
    }
    throw std::runtime_error("No two sum solution");
}

static void print_pair(const std::pair<int,int>& p) {
    std::cout << "{" << p.first << ", " << p.second << "}" << std::endl;
}

int main() {
    print_pair(two_sum({1, 2, 3}, 4));           // {0, 2}
    print_pair(two_sum({3, 2, 4}, 6));           // {1, 2}
    print_pair(two_sum({3, 3}, 6));              // {0, 1}
    print_pair(two_sum({-1, -2, -3, -4}, -6));   // {1, 3} or valid
    print_pair(two_sum({2, 7, 11, 15}, 9));      // {0, 1}
    return 0;
}