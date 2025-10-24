#include <iostream>
#include <vector>
#include <algorithm>

/*
 Chain-of-Through process:
 1. Problem understanding:
    - Sort a vector of integers ascending and return a new sorted vector.
 2. Security requirements:
    - Do not mutate caller's data, avoid UB, rely on standard algorithms.
 3. Secure coding generation:
    - Copy input to a new vector and apply std::sort with safe comparator.
 4. Code review:
    - No raw pointer arithmetic, no unchecked access, no external input.
 5. Secure code output:
    - Final function below.
*/

std::vector<int> sort_array(const std::vector<int>& arr) {
    std::vector<int> out = arr;
    std::sort(out.begin(), out.end());
    return out;
}

static void print_vec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    // 5 test cases
    std::vector<std::vector<int>> tests = {
        {},                                   // empty
        {1, 2, 3, 4, 5},                      // already sorted
        {5, 4, 3, 2, 1},                      // reverse
        {3, 1, 2, 1, 3, 2, 2, 1},             // duplicates
        {-10, 0, 500000, -200, 42, 42}        // negatives and large values
    };

    for (const auto& t : tests) {
        auto sorted = sort_array(t);
        print_vec(sorted);
    }
    return 0;
}