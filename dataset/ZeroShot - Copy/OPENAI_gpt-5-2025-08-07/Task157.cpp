#include <iostream>
#include <vector>
#include <memory>
#include <limits>
#include <stdexcept>

long long allocate_and_process(const std::vector<int>& data) {
    const size_t n = data.size();

    // Check for multiplication overflow for demonstration (although new[] uses element count)
    if (n > (std::numeric_limits<size_t>::max() / sizeof(int))) {
        throw std::length_error("Requested allocation size is too large.");
    }

    // Allocate dynamically and ensure deallocation via unique_ptr
    std::unique_ptr<int[]> arr(new (std::nothrow) int[n]);
    if (!arr && n != 0) {
        throw std::bad_alloc();
    }

    // Copy data and compute sum from allocated memory
    for (size_t i = 0; i < n; ++i) {
        arr[i] = data[i];
    }

    long long sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += static_cast<long long>(arr[i]);
    }

    // arr automatically deallocated here
    return sum;
}

int main() {
    // 5 test cases
    std::vector<std::vector<int>> tests = {
        {},                                        // sum = 0
        {1, 2, 3, 4, 5},                           // sum = 15
        {2147483647, 1, -2},                       // sum = 2147483646
        [](){ std::vector<int> v; v.reserve(1000);
              for (int i = -500; i <= 499; ++i) v.push_back(i);
              return v; }(),                       // sum = -500
        {1000000000, 1000000000, 1000000000}       // sum = 3000000000
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            long long result = allocate_and_process(tests[i]);
            std::cout << "Test " << (i + 1) << " sum = " << result << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Test " << (i + 1) << " error: " << ex.what() << "\n";
        }
    }
    return 0;
}