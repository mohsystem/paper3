#include <iostream>
#include <vector>

/**
 * @brief Finds the outlier in an array of integers.
 * 
 * The array is guaranteed to have a length of at least 3 and is either
 * entirely comprised of odd integers or entirely comprised of even integers
 * except for a single integer N. This function finds and returns N.
 * 
 * @param arr A constant reference to a vector of integers.
 * @return The outlier integer N.
 */
int findOutlier(const std::vector<int>& arr) {
    // Determine the majority parity by checking the first three elements.
    // The sum of booleans (true=1, false=0) tells us the count of even numbers.
    int even_count = (arr[0] % 2 == 0) + (arr[1] % 2 == 0) + (arr[2] % 2 == 0);
    bool majority_is_even = even_count >= 2;

    // Iterate through the array to find the number with the opposite parity.
    for (int num : arr) {
        bool current_is_even = (num % 2 == 0);
        if (current_is_even != majority_is_even) {
            return num;
        }
    }

    // This part should be unreachable given the problem constraints.
    // Return 0 as a fallback.
    return 0;
}

int main() {
    // Test Case 1: Outlier is odd
    std::vector<int> test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
    std::cout << "Test 1 Outlier: " << findOutlier(test1) << std::endl;

    // Test Case 2: Outlier is even
    std::vector<int> test2 = {160, 3, 1719, 19, 11, 13, -21};
    std::cout << "Test 2 Outlier: " << findOutlier(test2) << std::endl;

    // Test Case 3: Outlier is odd, at the end
    std::vector<int> test3 = {2, 6, 8, -10, 3};
    std::cout << "Test 3 Outlier: " << findOutlier(test3) << std::endl;

    // Test Case 4: Outlier is even, at the end
    std::vector<int> test4 = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 20};
    std::cout << "Test 4 Outlier: " << findOutlier(test4) << std::endl;

    // Test Case 5: Outlier is even (zero), at the end
    std::vector<int> test5 = {1, 3, 5, 7, 9, 11, 0};
    std::cout << "Test 5 Outlier: " << findOutlier(test5) << std::endl;

    return 0;
}