
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

class Task143 {
public:
    /**
     * Sorts a vector of integers in ascending order
     * @param arr The input vector to be sorted
     * @return A new sorted vector (original vector remains unchanged)
     */
    static std::vector<int> sortArray(const std::vector<int>& arr) {
        // Create a copy to avoid modifying the original vector
        std::vector<int> result = arr;
        
        // Use std::sort which is secure and efficient
        std::sort(result.begin(), result.end());
        
        return result;
    }
    
    /**
     * Helper function to print vector
     */
    static void printVector(const std::vector<int>& vec) {
        std::cout << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            std::cout << vec[i];
            if (i < vec.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
};

int main() {
    // Test case 1: Normal array
    std::vector<int> test1 = {5, 2, 8, 1, 9};
    std::cout << "Test 1: ";
    Task143::printVector(Task143::sortArray(test1));
    
    // Test case 2: Already sorted array
    std::vector<int> test2 = {1, 2, 3, 4, 5};
    std::cout << "Test 2: ";
    Task143::printVector(Task143::sortArray(test2));
    
    // Test case 3: Reverse sorted array
    std::vector<int> test3 = {9, 7, 5, 3, 1};
    std::cout << "Test 3: ";
    Task143::printVector(Task143::sortArray(test3));
    
    // Test case 4: Array with duplicates
    std::vector<int> test4 = {4, 2, 7, 2, 9, 4};
    std::cout << "Test 4: ";
    Task143::printVector(Task143::sortArray(test4));
    
    // Test case 5: Empty array
    std::vector<int> test5 = {};
    std::cout << "Test 5: ";
    Task143::printVector(Task143::sortArray(test5));
    
    return 0;
}
