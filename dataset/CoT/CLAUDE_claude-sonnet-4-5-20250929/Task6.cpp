
#include <iostream>
#include <vector>
#include <stdexcept>

class Task6 {
public:
    static double findUniq(const std::vector<double>& arr) {
        // Security: Validate input array
        if (arr.size() < 3) {
            throw std::invalid_argument("Array must contain at least 3 elements");
        }
        
        // Compare first three elements to determine which is unique
        if (arr[0] == arr[1]) {
            // arr[0] and arr[1] are the same
            for (size_t i = 2; i < arr.size(); i++) {
                if (arr[i] != arr[0]) {
                    return arr[i];
                }
            }
            return arr[2];
        } else if (arr[0] == arr[2]) {
            // arr[0] and arr[2] are the same, arr[1] is unique
            return arr[1];
        } else {
            // arr[1] and arr[2] must be the same, arr[0] is unique
            return arr[0];
        }
    }
};

int main() {
    // Test case 1
    std::cout << "Test 1: " << Task6::findUniq({1, 1, 1, 2, 1, 1}) << std::endl;  // Expected: 2
    
    // Test case 2
    std::cout << "Test 2: " << Task6::findUniq({0, 0, 0.55, 0, 0}) << std::endl;  // Expected: 0.55
    
    // Test case 3
    std::cout << "Test 3: " << Task6::findUniq({5, 5, 5, 5, 3}) << std::endl;  // Expected: 3
    
    // Test case 4
    std::cout << "Test 4: " << Task6::findUniq({10, 20, 20, 20}) << std::endl;  // Expected: 10
    
    // Test case 5
    std::cout << "Test 5: " << Task6::findUniq({-1, -1, -1, -1, 0}) << std::endl;  // Expected: 0
    
    return 0;
}
