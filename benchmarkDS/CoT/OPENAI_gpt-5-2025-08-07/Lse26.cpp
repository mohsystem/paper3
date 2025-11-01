#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

long long getMonthlySales(const std::vector<long long>& sales, std::size_t i) {
    if (i >= sales.size()) {
        throw std::invalid_argument("Index out of bounds for sales array");
    }
    return sales[i];
}

long long totalFirstQuarterSales(const std::vector<long long>& sales) {
    if (sales.size() < 3) {
        throw std::invalid_argument("Sales array must have at least 3 elements");
    }
    long long sum = 0;
    for (std::size_t i = 0; i < 3; ++i) {
        __int128 tmp = static_cast<__int128>(sum) + static_cast<__int128>(getMonthlySales(sales, i));
        if (tmp > std::numeric_limits<long long>::max() || tmp < std::numeric_limits<long long>::min()) {
            throw std::overflow_error("Overflow during summation");
        }
        sum = static_cast<long long>(tmp);
    }
    return sum;
}

int main() {
    std::vector<std::vector<long long>> tests = {
        {100, 200, 300, 400, 500},                   // Expected: 600
        {0, 0, 0},                                   // Expected: 0
        {12345, 67890, 11111},                       // Expected: 91346
        {std::numeric_limits<long long>::max(), 0, 0, 5}, // Expected: LLONG_MAX
        {-10, 20, -5, 0, 1000}                       // Expected: 5
    };

    for (std::size_t t = 0; t < tests.size(); ++t) {
        try {
            long long result = totalFirstQuarterSales(tests[t]);
            std::cout << "Test " << (t + 1) << " [" << tests[t][0] << ", " << tests[t][1] << ", " << tests[t][2] << "] => " << result << "\n";
        } catch (const std::exception& e) {
            std::cout << "Test " << (t + 1) << " error: " << e.what() << "\n";
        }
    }

    return 0;
}