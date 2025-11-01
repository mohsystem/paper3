#include <iostream>
#include <vector>
#include <numeric>

// Using long long for sales data to prevent integer overflow.
using SalesType = long long;

/**
 * @brief Retrieves the sales for a specific month from a data source.
 * Secure: Performs bounds checking to prevent accessing out-of-bounds memory.
 * @param monthIndex The index of the month (0-2).
 * @param salesData A vector containing monthly sales data.
 * @return The sales for the given month, or 0 if the index is invalid.
 */
SalesType getMonthlySales(int monthIndex, const std::vector<SalesType>& salesData) {
    if (monthIndex < 0 || static_cast<size_t>(monthIndex) >= salesData.size()) {
        // Handle invalid input gracefully.
        return 0;
    }
    return salesData[static_cast<size_t>(monthIndex)];
}

/**
 * @brief Calculates the total sales for the first quarter (first 3 months).
 * The for loop iterates 3 times. On each iteration, it calls getMonthlySales
 * with the current loop index i. The return value is added to a running sum.
 * @param firstQuarterSalesData A vector with at least 3 elements representing sales.
 * @return The sum of the first three months' sales.
 */
SalesType calculateFirstQuarterSales(const std::vector<SalesType>& firstQuarterSalesData) {
    // Use long long for sum to prevent potential integer overflow.
    SalesType sum = 0;
    
    // The loop iterates 3 times for the first quarter (months 0, 1, 2).
    for (int i = 0; i < 3; ++i) {
        // On each iteration, get the sales for the month and add to sum.
        sum += getMonthlySales(i, firstQuarterSalesData);
    }
    
    return sum;
}

int main() {
    // 5 test cases to validate the functionality.
    std::vector<std::vector<SalesType>> testCases = {
        {10000LL, 12000LL, 15000LL},             // Standard case
        {0LL, 0LL, 0LL},                         // All zero sales
        {9000000000000000000LL, 10LL, 20LL},     // Large value test to check overflow prevention
        {1500LL, 2500LL, 500LL},                 // Small values
        {-100LL, 500LL, -200LL}                  // Edge case with negative values (e.g., returns)
    };

    std::cout << "Running 5 C++ test cases..." << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        const auto& currentCase = testCases[i];
        SalesType totalSales = calculateFirstQuarterSales(currentCase);
        std::cout << "Test Case " << i + 1 << ": [" << currentCase[0]
                  << ", " << currentCase[1] << ", " << currentCase[2]
                  << "] -> Total Sales: " << totalSales << std::endl;
    }

    return 0;
}