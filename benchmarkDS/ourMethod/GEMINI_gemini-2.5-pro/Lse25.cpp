#include <iostream>
#include <vector>
#include <random>
#include <numeric>

const int MAX_SALES = 100000;
const int MONTHS_IN_YEAR = 12;

/**
 * Generates a random monthly sales figure using a modern C++ random engine.
 * @param gen A reference to the random number generator engine.
 * @param dist A reference to the uniform integer distribution.
 * @return A random integer between 0 and 100000 (inclusive).
 */
int getMonthlySales(std::mt19937& gen, std::uniform_int_distribution<>& dist) {
    return dist(gen);
}

/**
 * Runs a single yearly simulation: gets 12 monthly sales,
 * calculates quarterly totals, and prints the results.
 * @param gen The random number generator engine to use.
 * @param dist The uniform integer distribution to use.
 */
void runSimulation(std::mt19937& gen, std::uniform_int_distribution<>& dist) {
    std::vector<int> monthlySales;
    monthlySales.reserve(MONTHS_IN_YEAR);

    // Get sales for each month
    for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
        monthlySales.push_back(getMonthlySales(gen, dist));
    }

    // Calculate and print quarterly sales.
    // Use long long to prevent potential integer overflow on sum.
    long long quarter1Sales = 0;
    long long quarter2Sales = 0;
    long long quarter3Sales = 0;
    long long quarter4Sales = 0;
    
    // Sum sales for each quarter
    for (int i = 0; i < 3; ++i) quarter1Sales += monthlySales[i];
    for (int i = 3; i < 6; ++i) quarter2Sales += monthlySales[i];
    for (int i = 6; i < 9; ++i) quarter3Sales += monthlySales[i];
    for (int i = 9; i < 12; ++i) quarter4Sales += monthlySales[i];

    std::cout << "Quarter 1 Sales: " << quarter1Sales << std::endl;
    std::cout << "Quarter 2 Sales: " << quarter2Sales << std::endl;
    std::cout << "Quarter 3 Sales: " << quarter3Sales << std::endl;
    std::cout << "Quarter 4 Sales: " << quarter4Sales << std::endl;
}

/**
 * Main function to run the simulation 5 times as test cases.
 */
int main() {
    // Seed the random number generator once with a high-quality entropy source
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, MAX_SALES);

    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---" << std::endl;
        runSimulation(gen, dist);
        if (i < 5) {
            std::cout << std::endl;
        }
    }

    return 0;
}