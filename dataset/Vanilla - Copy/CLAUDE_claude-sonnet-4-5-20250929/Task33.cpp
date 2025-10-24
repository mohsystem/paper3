
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

int queueTime(std::vector<int> customers, int n) {
    if (customers.empty() || n <= 0) {
        return 0;
    }
    
    if (n >= customers.size()) {
        return *std::max_element(customers.begin(), customers.end());
    }
    
    // Use a min heap to track till finish times
    std::priority_queue<int, std::vector<int>, std::greater<int>> tills;
    
    // Initialize all tills with 0
    for (int i = 0; i < n; i++) {
        tills.push(0);
    }
    
    // Process each customer
    for (int customerTime : customers) {
        // Get the till that will be free first
        int earliestFree = tills.top();
        tills.pop();
        // Add customer time to that till
        tills.push(earliestFree + customerTime);
    }
    
    // Find the maximum time (last till to finish)
    int maxTime = 0;
    while (!tills.empty()) {
        maxTime = std::max(maxTime, tills.top());
        tills.pop();
    }
    
    return maxTime;
}

int main() {
    // Test case 1
    std::cout << "Test 1: " << queueTime({5, 3, 4}, 1) << " (Expected: 12)" << std::endl;
    
    // Test case 2
    std::cout << "Test 2: " << queueTime({10, 2, 3, 3}, 2) << " (Expected: 10)" << std::endl;
    
    // Test case 3
    std::cout << "Test 3: " << queueTime({2, 3, 10}, 2) << " (Expected: 12)" << std::endl;
    
    // Test case 4
    std::cout << "Test 4: " << queueTime({1, 2, 3, 4, 5}, 3) << " (Expected: 6)" << std::endl;
    
    // Test case 5
    std::cout << "Test 5: " << queueTime({}, 1) << " (Expected: 0)" << std::endl;
    
    return 0;
}
