
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
    
    // Use a min heap to track the finish time of each till
    std::priority_queue<int, std::vector<int>, std::greater<int>> tills;
    
    // Initialize all tills with 0
    for (int i = 0; i < n; i++) {
        tills.push(0);
    }
    
    // Process each customer
    for (int customerTime : customers) {
        // Get the till that will be free first
        int earliestFreeTill = tills.top();
        tills.pop();
        // Add the customer to that till
        tills.push(earliestFreeTill + customerTime);
    }
    
    // The total time is when the last till finishes
    int maxTime = 0;
    while (!tills.empty()) {
        maxTime = std::max(maxTime, tills.top());
        tills.pop();
    }
    
    return maxTime;
}

int main() {
    // Test case 1
    std::vector<int> test1 = {5, 3, 4};
    std::cout << "Test 1: " << queueTime(test1, 1) << " (Expected: 12)" << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {10, 2, 3, 3};
    std::cout << "Test 2: " << queueTime(test2, 2) << " (Expected: 10)" << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {2, 3, 10};
    std::cout << "Test 3: " << queueTime(test3, 2) << " (Expected: 12)" << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {};
    std::cout << "Test 4: " << queueTime(test4, 1) << " (Expected: 0)" << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {1, 2, 3, 4, 5};
    std::cout << "Test 5: " << queueTime(test5, 100) << " (Expected: 5)" << std::endl;
    
    return 0;
}
