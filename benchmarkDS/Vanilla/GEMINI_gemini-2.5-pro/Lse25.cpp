#include <iostream>
#include <random>
#include <chrono>

/**
 * @brief Returns a random number between 0 and 100000.
 * 
 * Uses a static random engine to ensure it's seeded only once
 * and provides different numbers on subsequent calls.
 * @return int A random integer representing monthly sales.
 */
int getMonthlySales() {
    static std::mt19937 engine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(0, 100000);
    return dist(engine);
}

/**
 * @brief Simulates a year of sales, calculates the total for each quarter,
 * and prints the results.
 */
void calculateAndPrintQuarterlySales() {
    int quarter1Sales = 0;
    int quarter2Sales = 0;
    int quarter3Sales = 0;
    int quarter4Sales = 0;

    // Loop through 12 months of the year
    for (int month = 1; month <= 12; ++month) {
        int monthlySale = getMonthlySales();
        if (month <= 3) {
            quarter1Sales += monthlySale;
        } else if (month <= 6) {
            quarter2Sales += monthlySale;
        } else if (month <= 9) {
            quarter3Sales += monthlySale;
        } else {
            quarter4Sales += monthlySale;
        }
    }

    // Print the results for each quarter
    std::cout << "Quarter 1 Sales: " << quarter1Sales << std::endl;
    std::cout << "Quarter 2 Sales: " << quarter2Sales << std::endl;
    std::cout << "Quarter 3 Sales: " << quarter3Sales << std::endl;
    std::cout << "Quarter 4 Sales: " << quarter4Sales << std::endl;
    std::cout << "--------------------" << std::endl;
}

int main() {
    // Run 5 test cases
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << std::endl;
        calculateAndPrintQuarterlySales();
    }
    return 0;
}