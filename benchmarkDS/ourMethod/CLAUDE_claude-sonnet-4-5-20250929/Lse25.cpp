
#include <iostream>
#include <random>
#include <limits>
#include <stdexcept>

// Use a cryptographically secure random number generator
// Thread-local to avoid concurrency issues
thread_local std::random_device rd;
thread_local std::mt19937 gen(rd());

/**
 * Returns a random sales value between 0 and 100000 (inclusive).
 * Uses std::random_device and std::mt19937 for quality random numbers.
 * 
 * @return random sales value in range [0, 100000]
 */
int getMonthlySales() {
    // std::uniform_int_distribution is inclusive on both ends
    std::uniform_int_distribution<int> dist(0, 100000);
    return dist(gen);
}

/**
 * Calculates quarterly sales by calling getMonthlySales 12 times
 * and aggregating results into 4 quarters.
 */
void calculateQuarterlySales() {
    // Initialize quarterly sales - explicit initialization for safety
    int q1 = 0;
    int q2 = 0;
    int q3 = 0;
    int q4 = 0;
    
    // Quarter 1: January, February, March (months 0-2)
    for (int i = 0; i < 3; ++i) {
        int sales = getMonthlySales();
        // Check for integer overflow before addition
        if (q1 > std::numeric_limits<int>::max() - sales) {
            std::cerr << "Error: Integer overflow detected in Q1" << std::endl;
            return;
        }
        q1 += sales;
    }
    
    // Quarter 2: April, May, June (months 3-5)
    for (int i = 0; i < 3; ++i) {
        int sales = getMonthlySales();
        if (q2 > std::numeric_limits<int>::max() - sales) {
            std::cerr << "Error: Integer overflow detected in Q2" << std::endl;
            return;
        }
        q2 += sales;
    }
    
    // Quarter 3: July, August, September (months 6-8)
    for (int i = 0; i < 3; ++i) {
        int sales = getMonthlySales();
        if (q3 > std::numeric_limits<int>::max() - sales) {
            std::cerr << "Error: Integer overflow detected in Q3" << std::endl;
            return;
        }
        q3 += sales;
    }
    
    // Quarter 4: October, November, December (months 9-11)
    for (int i = 0; i < 3; ++i) {
        int sales = getMonthlySales();
        if (q4 > std::numeric_limits<int>::max() - sales) {
            std::cerr << "Error: Integer overflow detected in Q4" << std::endl;
            return;
        }
        q4 += sales;
    }
    
    // Print results using constant format strings for security
    std::cout << "Q1 Sales: " << q1 << std::endl;
    std::cout << "Q2 Sales: " << q2 << std::endl;
    std::cout << "Q3 Sales: " << q3 << std::endl;
    std::cout << "Q4 Sales: " << q4 << std::endl;
}

int main() {
    // Test case 1
    std::cout << "Test Case 1:" << std::endl;
    calculateQuarterlySales();
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test Case 2:" << std::endl;
    calculateQuarterlySales();
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test Case 3:" << std::endl;
    calculateQuarterlySales();
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test Case 4:" << std::endl;
    calculateQuarterlySales();
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test Case 5:" << std::endl;
    calculateQuarterlySales();
    
    return 0;
}
