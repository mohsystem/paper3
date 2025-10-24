#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

/**
 * @brief Calculates the total time required for all customers to check out.
 * 
 * @param customers A vector of positive integers representing the time each customer needs.
 * @param n The number of checkout tills.
 * @return The total time required.
 */
long long queueTime(const std::vector<int>& customers, int n) {
    if (customers.empty()) {
        return 0;
    }

    // Use long long to prevent potential integer overflow
    std::vector<long long> tills(n, 0);

    for (int customerTime : customers) {
        // Find the till that will be free the soonest
        // std::min_element returns an iterator to the smallest element
        auto minTillIt = std::min_element(tills.begin(), tills.end());
        
        // Assign the next customer to this till by dereferencing the iterator
        *minTillIt += customerTime;
    }

    // The total time is the time the last till becomes free
    // std::max_element returns an iterator to the largest element
    return *std::max_element(tills.begin(), tills.end());
}

int main() {
    // Test Case 1
    std::cout << queueTime(std::vector<int>{5, 3, 4}, 1) << std::endl; // Expected: 12

    // Test Case 2
    std::cout << queueTime(std::vector<int>{10, 2, 3, 3}, 2) << std::endl; // Expected: 10

    // Test Case 3
    std::cout << queueTime(std::vector<int>{2, 3, 10}, 2) << std::endl; // Expected: 12
    
    // Test Case 4 (empty queue)
    std::cout << queueTime(std::vector<int>{}, 1) << std::endl; // Expected: 0

    // Test Case 5 (more tills than customers)
    std::cout << queueTime(std::vector<int>{1, 2, 3, 4, 5}, 100) << std::endl; // Expected: 5

    return 0;
}