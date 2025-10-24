#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

class Task33 {
public:
    static int queueTime(const std::vector<int>& customers, int n) {
        if (customers.empty()) {
            return 0;
        }

        std::vector<int> tills(n, 0);

        for (int customerTime : customers) {
            // Find the till that will be free earliest
            // std::min_element returns an iterator to the first element with the minimum value
            auto minTillIt = std::min_element(tills.begin(), tills.end());
            
            // Assign the customer to this till by adding their time
            *minTillIt += customerTime;
        }

        // The total time is the time the last customer finishes, which is the max time in any till
        return *std::max_element(tills.begin(), tills.end());
    }
};

int main() {
    // Test Case 1
    std::cout << Task33::queueTime({5, 3, 4}, 1) << std::endl;
    // Test Case 2
    std::cout << Task33::queueTime({10, 2, 3, 3}, 2) << std::endl;
    // Test Case 3
    std::cout << Task33::queueTime({2, 3, 10}, 2) << std::endl;
    // Test Case 4
    std::cout << Task33::queueTime({}, 1) << std::endl;
    // Test Case 5
    std::cout << Task33::queueTime({1, 2, 3, 4, 5}, 100) << std::endl;

    return 0;
}