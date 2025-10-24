
#include <iostream>
#include <vector>
#include <algorithm>

class Task33 {
public:
    static int queueTime(std::vector<int> customers, int n) {
        // Input validation
        if (customers.empty() || n <= 0) {
            return 0;
        }
        
        // If number of tills is greater than or equal to customers,
        // return the maximum time among all customers
        if (n >= static_cast<int>(customers.size())) {
            return *std::max_element(customers.begin(), customers.end());
        }
        
        // Create a vector to represent the tills and their busy time
        std::vector<int> tills(n, 0);
        
        // Assign each customer to the till that will be free first
        for (int customerTime : customers) {
            // Find the till with minimum time
            int minIndex = 0;
            int minTime = tills[0];
            for (int i = 1; i < n; i++) {
                if (tills[i] < minTime) {
                    minTime = tills[i];
                    minIndex = i;
                }
            }
            // Assign customer to the till with minimum time
            tills[minIndex] += customerTime;
        }
        
        // Find and return the maximum time among all tills
        return *std::max_element(tills.begin(), tills.end());
    }
};

int main() {
    // Test case 1
    std::vector<int> customers1 = {5, 3, 4};
    std::cout << "Test 1: " << Task33::queueTime(customers1, 1) << std::endl; // Expected: 12
    
    // Test case 2
    std::vector<int> customers2 = {10, 2, 3, 3};
    std::cout << "Test 2: " << Task33::queueTime(customers2, 2) << std::endl; // Expected: 10
    
    // Test case 3
    std::vector<int> customers3 = {2, 3, 10};
    std::cout << "Test 3: " << Task33::queueTime(customers3, 2) << std::endl; // Expected: 12
    
    // Test case 4
    std::vector<int> customers4 = {};
    std::cout << "Test 4: " << Task33::queueTime(customers4, 1) << std::endl; // Expected: 0
    
    // Test case 5
    std::vector<int> customers5 = {1, 2, 3, 4, 5};
    std::cout << "Test 5: " << Task33::queueTime(customers5, 5) << std::endl; // Expected: 5
    
    return 0;
}
