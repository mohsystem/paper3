
#include <iostream>
#include <vector>

double findUniq(const std::vector<double>& arr) {
    // Compare first three elements to determine which is unique
    if (arr[0] == arr[1]) {
        // arr[0] is the common element
        double common = arr[0];
        for (double num : arr) {
            if (num != common) {
                return num;
            }
        }
    } else if (arr[0] == arr[2]) {
        // arr[0] is the common element, arr[1] is unique
        return arr[1];
    } else {
        // arr[1] or arr[2] is the common element, arr[0] is unique
        return arr[0];
    }
    return arr[0]; // Should not reach here
}

int main() {
    // Test case 1
    std::cout << findUniq({1, 1, 1, 2, 1, 1}) << std::endl; // => 2
    
    // Test case 2
    std::cout << findUniq({0, 0, 0.55, 0, 0}) << std::endl; // => 0.55
    
    // Test case 3
    std::cout << findUniq({3, 3, 3, 3, 5}) << std::endl; // => 5
    
    // Test case 4
    std::cout << findUniq({10, 9, 9, 9, 9}) << std::endl; // => 10
    
    // Test case 5
    std::cout << findUniq({7.5, 7.5, 8.5, 7.5}) << std::endl; // => 8.5
    
    return 0;
}
