
#include <iostream>
#include <vector>
#include <algorithm>

class Task163 {
public:
    static int longestRun(const std::vector<int>& arr) {
        if (arr.empty()) {
            return 0;
        }
        
        if (arr.size() == 1) {
            return 1;
        }
        
        int maxRun = 1;
        int currentRun = 1;
        
        for (size_t i = 1; i < arr.size(); i++) {
            int diff = arr[i] - arr[i - 1];
            
            if (diff == 1 || diff == -1) {
                if (i > 1) {
                    int prevDiff = arr[i - 1] - arr[i - 2];
                    if (diff == prevDiff) {
                        currentRun++;
                    } else {
                        currentRun = 2;
                    }
                } else {
                    currentRun++;
                }
                maxRun = std::max(maxRun, currentRun);
            } else {
                currentRun = 1;
            }
        }
        
        return maxRun;
    }
};

int main() {
    // Test case 1
    std::vector<int> test1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "Test 1: " << Task163::longestRun(test1) << " (Expected: 5)" << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {1, 2, 3, 10, 11, 15};
    std::cout << "Test 2: " << Task163::longestRun(test2) << " (Expected: 3)" << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {5, 4, 2, 1};
    std::cout << "Test 3: " << Task163::longestRun(test3) << " (Expected: 2)" << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {3, 5, 7, 10, 15};
    std::cout << "Test 4: " << Task163::longestRun(test4) << " (Expected: 1)" << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {10, 9, 8, 7, 6, 5};
    std::cout << "Test 5: " << Task163::longestRun(test5) << " (Expected: 6)" << std::endl;
    
    return 0;
}
