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
int queueTime(const std::vector<int>& customers, int n) {
    if (customers.empty()) {
        return 0;
    }

    // A vector to represent the total time accumulated at each till.
    // Initialized to 0 for all n tills.
    std::vector<int> tills(n, 0);

    // Iterate through each customer in the queue.
    for (int customer_time : customers) {
        // Find the till that will become free the soonest.
        // std::min_element returns an iterator to the first element with the minimum value.
        auto soonest_free_till = std::min_element(tills.begin(), tills.end());
        
        // Assign the current customer to this till by adding their time.
        *soonest_free_till += customer_time;
    }

    // The total time is determined by the till that finishes last.
    // This is the maximum value in the tills vector.
    auto last_till_to_finish = std::max_element(tills.begin(), tills.end());
    
    // Dereference the iterator to get the maximum time.
    return *last_till_to_finish;
}

int main() {
    // Test Case 1
    std::vector<int> c1 = {5, 3, 4};
    std::cout << "Test 1: queueTime({5, 3, 4}, 1) -> " << queueTime(c1, 1) << " (Expected: 12)" << std::endl;

    // Test Case 2
    std::vector<int> c2 = {10, 2, 3, 3};
    std::cout << "Test 2: queueTime({10, 2, 3, 3}, 2) -> " << queueTime(c2, 2) << " (Expected: 10)" << std::endl;

    // Test Case 3
    std::vector<int> c3 = {2, 3, 10};
    std::cout << "Test 3: queueTime({2, 3, 10}, 2) -> " << queueTime(c3, 2) << " (Expected: 12)" << std::endl;

    // Test Case 4 (empty queue)
    std::vector<int> c4 = {};
    std::cout << "Test 4: queueTime({}, 1) -> " << queueTime(c4, 1) << " (Expected: 0)" << std::endl;

    // Test Case 5 (more tills than customers)
    std::vector<int> c5 = {1, 2, 3, 4, 5};
    std::cout << "Test 5: queueTime({1, 2, 3, 4, 5}, 100) -> " << queueTime(c5, 100) << " (Expected: 5)" << std::endl;

    return 0;
}