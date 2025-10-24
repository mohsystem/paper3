#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

class Task33 {
public:
    /**
     * Calculates the total time required for all customers to check out.
     *
     * @param customers A vector of positive integers representing the queue.
     * @param n The number of checkout tills.
     * @return The total time required.
     */
    static int queueTime(const std::vector<int>& customers, int n) {
        if (customers.empty()) {
            return 0;
        }

        std::vector<int> tills(n, 0);

        for (int customerTime : customers) {
            // Find the till that will be free the soonest and assign the customer
            *std::min_element(tills.begin(), tills.end()) += customerTime;
        }

        // The total time is the time the last till becomes free
        return *std::max_element(tills.begin(), tills.end());
    }
};

int main() {
    // Test cases
    std::cout << Task33::queueTime({}, 1) << std::endl; // Expected: 0
    std::cout << Task33::queueTime({5, 3, 4}, 1) << std::endl; // Expected: 12
    std::cout << Task33::queueTime({10, 2, 3, 3}, 2) << std::endl; // Expected: 10
    std::cout << Task33::queueTime({2, 3, 10}, 2) << std::endl; // Expected: 12
    std::cout << Task33::queueTime({1, 2, 3, 4, 5}, 100) << std::endl; // Expected: 5
    return 0;
}