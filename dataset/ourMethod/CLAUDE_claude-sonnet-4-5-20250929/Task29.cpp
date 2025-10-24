
#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <limits>

// Function to find two indices where numbers sum to target
// Uses hash map for O(n) time complexity
// Security: validates input size, checks for integer overflow, uses safe containers
std::pair<int, int> two_sum(const std::vector<int>& numbers, int target) {
    // Validate input size (must be at least 2 elements as per problem statement)
    if (numbers.size() < 2) {
        throw std::invalid_argument("Array must contain at least 2 elements");
    }
    
    // Check for unreasonably large input to prevent resource exhaustion
    if (numbers.size() > 1000000) {
        throw std::invalid_argument("Array size exceeds safe limit");
    }
    
    // Use unordered_map to store value -> index mapping
    // This provides O(1) average lookup time
    std::unordered_map<int, size_t> seen;
    
    // Iterate through array once
    for (size_t i = 0; i < numbers.size(); ++i) {
        int current = numbers[i];
        
        // Calculate complement needed to reach target
        // Check for integer overflow before subtraction
        long long complement_ll = static_cast<long long>(target) - static_cast<long long>(current);
        
        // Validate complement is within int range
        if (complement_ll < std::numeric_limits<int>::min() || 
            complement_ll > std::numeric_limits<int>::max()) {
            continue; // Skip this iteration if overflow would occur
        }
        
        int complement = static_cast<int>(complement_ll);
        
        // Check if complement exists in our seen map
        auto it = seen.find(complement);
        if (it != seen.end()) {
            // Found a pair: return indices (earlier index first)
            return std::make_pair(static_cast<int>(it->second), static_cast<int>(i));
        }
        
        // Store current number and its index for future lookups
        // Only store if not already present (keep first occurrence)
        if (seen.find(current) == seen.end()) {
            seen[current] = i;
        }
    }
    
    // Per problem statement, input always has valid solution
    // This should never be reached with valid input
    throw std::runtime_error("No solution found");
}

int main() {
    // Test case 1: Basic example from problem
    std::vector<int> test1 = {1, 2, 3};
    auto result1 = two_sum(test1, 4);
    std::cout << "Test 1: [1,2,3], target=4 -> (" << result1.first << ", " << result1.second << ")" << std::endl;
    
    // Test case 2: Second example from problem
    std::vector<int> test2 = {3, 2, 4};
    auto result2 = two_sum(test2, 6);
    std::cout << "Test 2: [3,2,4], target=6 -> (" << result2.first << ", " << result2.second << ")" << std::endl;
    
    // Test case 3: Negative numbers
    std::vector<int> test3 = {-1, -2, -3, -4, -5};
    auto result3 = two_sum(test3, -8);
    std::cout << "Test 3: [-1,-2,-3,-4,-5], target=-8 -> (" << result3.first << ", " << result3.second << ")" << std::endl;
    
    // Test case 4: Mix of positive and negative
    std::vector<int> test4 = {5, -2, 8, -7, 3};
    auto result4 = two_sum(test4, 1);
    std::cout << "Test 4: [5,-2,8,-7,3], target=1 -> (" << result4.first << ", " << result4.second << ")" << std::endl;
    
    // Test case 5: Large numbers
    std::vector<int> test5 = {1000000, 500000, 250000, 750000};
    auto result5 = two_sum(test5, 1500000);
    std::cout << "Test 5: [1000000,500000,250000,750000], target=1500000 -> (" << result5.first << ", " << result5.second << ")" << std::endl;
    
    return 0;
}
