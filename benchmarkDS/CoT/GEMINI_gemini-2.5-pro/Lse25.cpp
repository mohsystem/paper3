#include <iostream>
#include <random>
#include <ctime>

/**
 * @brief Returns a random sales figure between 0 and 100000, inclusive.
 * 
 * Uses a static random number generator to ensure it's seeded only once
 * and provides different numbers on subsequent calls.
 * @return An integer representing the monthly sales.
 */
int getMonthlySales() {
    static std::mt19937 generator(time(0));
    static std::uniform_int_distribution<int> distribution(0, 100000);
    return distribution(generator);
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---" << std::endl;
        
        int quarter1Sales = 0;
        int quarter2Sales = 0;
        int quarter3Sales = 0;
        int quarter4Sales = 0;

        // Call getMonthlySales 12 times, once for each month
        for (int month = 1; month <= 12; ++month) {
            int sales = getMonthlySales();
            if (month <= 3) {
                quarter1Sales += sales;
            } else if (month <= 6) {
                quarter2Sales += sales;
            } else if (month <= 9) {
                quarter3Sales += sales;
            } else {
                quarter4Sales += sales;
            }
        }

        // Print the results for each quarter
        std::cout << "Quarter 1 Sales: " << quarter1Sales << std::endl;
        std::cout << "Quarter 2 Sales: " << quarter2Sales << std::endl;
        std::cout << "Quarter 3 Sales: " << quarter3Sales << std::endl;
        std::cout << "Quarter 4 Sales: " << quarter4Sales << std::endl;
        std::cout << std::endl;
    }
    return 0;
}