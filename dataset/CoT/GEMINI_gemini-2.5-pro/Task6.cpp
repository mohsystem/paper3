#include <iostream>
#include <vector>

class Task6 {
public:
    static double findUniq(const std::vector<double>& arr) {
        // It's guaranteed that array contains at least 3 numbers.
        // If the first two numbers are different, one of them is the unique number.
        // We can check the third number to decide which one.
        if (arr[0] != arr[1]) {
            // If arr[0] is equal to arr[2], then arr[1] must be the unique number.
            // Otherwise, arr[0] is the unique number.
            return arr[0] == arr[2] ? arr[1] : arr[0];
        } else {
            // If the first two numbers are the same, this is the common number.
            // We can then iterate through the rest of the array to find the different one.
            double commonNumber = arr[0];
            for (size_t i = 2; i < arr.size(); ++i) {
                if (arr[i] != commonNumber) {
                    return arr[i];
                }
            }
        }
        // This part should be unreachable given the problem constraints.
        return -1.0;
    }
};

int main() {
    // Test Case 1
    std::cout << Task6::findUniq({1, 1, 1, 2, 1, 1}) << std::endl; // Expected: 2
    // Test Case 2
    std::cout << Task6::findUniq({0, 0, 0.55, 0, 0}) << std::endl; // Expected: 0.55
    // Test Case 3
    std::cout << Task6::findUniq({3, 10, 3, 3, 3}) << std::endl; // Expected: 10
    // Test Case 4
    std::cout << Task6::findUniq({5, 5, 5, 5, 4}) << std::endl; // Expected: 4
    // Test Case 5
    std::cout << Task6::findUniq({8, 8, 8, 8, 8, 8, 8, 7}) << std::endl; // Expected: 7
    return 0;
}