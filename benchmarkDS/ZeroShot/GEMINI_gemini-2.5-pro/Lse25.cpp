#include <iostream>
#include <random> // For std::random_device, std::mt19937, std::uniform_int_distribution
#include <vector>

// Note: Using the C++ <random> library is more secure and provides better
// statistical properties than the older C-style rand().
int getMonthlySales() {
    // A static random engine ensures it's seeded only once, improving performance
    // and randomness quality over re-seeding in each call.
    static std::random_device rd; // Will be used to obtain a seed for the random number engine
    static std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    static std::uniform_int_distribution<> distrib(0, 100000);

    return distrib(gen);
}

void calculateAndPrintQuarterlySales() {
    // Use a vector to store sales for each quarter.
    // Initialized to size 4, with all elements as 0.
    std::vector<long> quarterly_sales(4, 0L);

    for (int month = 1; month <= 12; ++month) {
        int sale = getMonthlySales();
        // Determine the quarter index (0-3)
        int quarter_index = (month - 1) / 3;
        quarterly_sales[quarter_index] += sale;
    }

    std::cout << "Quarter 1 Sales: $" << quarterly_sales[0] << std::endl;
    std::cout << "Quarter 2 Sales: $" << quarterly_sales[1] << std::endl;
    std::cout << "Quarter 3 Sales: $" << quarterly_sales[2] << std::endl;
    std::cout << "Quarter 4 Sales: $" << quarterly_sales[3] << std::endl;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---" << std::endl;
        calculateAndPrintQuarterlySales();
        std::cout << std::endl;
    }
    return 0;
}