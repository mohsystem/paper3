
#include <iostream>
#include <vector>
#include <limits>
#include <stdexcept>

// Finds the contiguous subarray with maximum sum using Kadane's algorithm\n// Returns a pair: {max_sum, {start_index, end_index}}\n// Security: All inputs validated, no buffer overflows, integer overflow checks\nstd::pair<long long, std::pair<size_t, size_t>> maxSubarraySum(const std::vector<int>& arr) {\n    // Input validation: empty array check\n    if (arr.empty()) {\n        throw std::invalid_argument("Array cannot be empty");\n    }\n    \n    // Input validation: size check to prevent integer overflow in calculations\n    if (arr.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {\n        throw std::invalid_argument("Array size too large");\n    }\n    \n    // Use long long to prevent integer overflow during sum calculations\n    long long max_sum = static_cast<long long>(arr[0]);\n    long long current_sum = static_cast<long long>(arr[0]);\n    \n    size_t start = 0;\n    size_t end = 0;\n    size_t temp_start = 0;\n    \n    // Kadane's algorithm with overflow protection
    for (size_t i = 1; i < arr.size(); ++i) {
        // Check for potential overflow before addition
        long long arr_val = static_cast<long long>(arr[i]);
        
        // If adding current element causes negative sum, restart from current element
        if (arr_val > current_sum + arr_val) {
            current_sum = arr_val;
            temp_start = i;
        } else {
            current_sum = current_sum + arr_val;
        }
        
        // Update maximum if current sum is greater
        if (current_sum > max_sum) {
            max_sum = current_sum;
            start = temp_start;
            end = i;
        }
    }
    
    return {max_sum, {start, end}};
}

int main() {
    // Test case 1: Normal array with positive and negative numbers
    try {
        std::vector<int> test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        auto result1 = maxSubarraySum(test1);
        std::cout << "Test 1: Max sum = " << result1.first 
                  << " (indices " << result1.second.first 
                  << " to " << result1.second.second << ")\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << "\\n";
    }
    
    // Test case 2: All negative numbers
    try {
        std::vector<int> test2 = {-5, -2, -8, -1, -4};
        auto result2 = maxSubarraySum(test2);
        std::cout << "Test 2: Max sum = " << result2.first 
                  << " (indices " << result2.second.first 
                  << " to " << result2.second.second << ")\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << "\\n";
    }
    
    // Test case 3: All positive numbers
    try {
        std::vector<int> test3 = {1, 2, 3, 4, 5};
        auto result3 = maxSubarraySum(test3);
        std::cout << "Test 3: Max sum = " << result3.first 
                  << " (indices " << result3.second.first 
                  << " to " << result3.second.second << ")\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << "\\n";
    }
    
    // Test case 4: Single element
    try {
        std::vector<int> test4 = {42};
        auto result4 = maxSubarraySum(test4);
        std::cout << "Test 4: Max sum = " << result4.first 
                  << " (indices " << result4.second.first 
                  << " to " << result4.second.second << ")\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << "\\n";
    }
    
    // Test case 5: Mix with zeros
    try {
        std::vector<int> test5 = {0, -3, 1, 1, 0, -2, 3};
        auto result5 = maxSubarraySum(test5);
        std::cout << "Test 5: Max sum = " << result5.first 
                  << " (indices " << result5.second.first 
                  << " to " << result5.second.second << ")\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 5 error: " << e.what() << "\\n";
    }
    
    return 0;
}
