
#include <iostream>
#include <vector>
#include <stdexcept>

class Task6 {
public:
    static double findUniq(const std::vector<double>& arr) {
        if (arr.size() < 3) {
            throw std::invalid_argument("Array must contain at least 3 numbers");
        }
        
        // Compare first three elements to determine which is the unique one
        if (arr[0] == arr[1]) {
            // arr[0] and arr[1] are the same, so they represent the repeated value
            for (size_t i = 2; i < arr.size(); i++) {
                if (arr[i] != arr[0]) {
                    return arr[i];
                }
            }
            return arr[0]; // Should not reach here given problem constraints
        } else if (arr[0] == arr[2]) {
            // arr[0] and arr[2] are the same, so arr[1] is unique
            return arr[1];
        } else {
            // arr[1] and arr[2] must be the same, so arr[0] is unique
            return arr[0];
        }
    }
};

int main() {
    // Test case 1
    double result1 = Task6::findUniq({1, 1, 1, 2, 1, 1});
    std::cout << "Test 1: " << result1 << " (Expected: 2.0)" << std::endl;
    
    // Test case 2
    double result2 = Task6::findUniq({0, 0, 0.55, 0, 0});
    std::cout << "Test 2: " << result2 << " (Expected: 0.55)" << std::endl;
    
    // Test case 3
    double result3 = Task6::findUniq({5, 5, 5, 5, 10});
    std::cout << "Test 3: " << result3 << " (Expected: 10.0)" << std::endl;
    
    // Test case 4
    double result4 = Task6::findUniq({3.14, 2.71, 2.71, 2.71});
    std::cout << "Test 4: " << result4 << " (Expected: 3.14)" << std::endl;
    
    // Test case 5
    double result5 = Task6::findUniq({-1, -1, -1, -5, -1, -1, -1});
    std::cout << "Test 5: " << result5 << " (Expected: -5.0)" << std::endl;
    
    return 0;
}
