#include <iostream>
#include <vector>
#include <numeric>

/**
 * @brief Determines if Alice wins the XOR game.
 * 
 * Alice wins if:
 * 1. The bitwise XOR of all elements is 0 at the start of her turn.
 * 2. The number of elements is even. If the initial XOR is not 0, she can always
 *    make a move to keep the XOR non-zero. Since the total number of turns is even,
 *    Bob will make the last move, making the XOR sum 0, and lose.
 * 
 * If the number of elements is odd and the initial XOR is not 0, Alice will make
 * the last move and lose.
 * 
 * @param nums A vector of integers on the chalkboard.
 * @return true if Alice wins, false otherwise.
 */
bool xorGame(const std::vector<int>& nums) {
    int xorSum = 0;
    for (int num : nums) {
        xorSum ^= num;
    }

    // Alice wins if the initial XOR sum is 0 (immediate win)
    // or if the number of elements is even.
    return xorSum == 0 || nums.size() % 2 == 0;
}

void print_vector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    // Test Case 1: Example 1, Expected: false
    std::vector<int> nums1 = {1, 1, 2};
    std::cout << "Input: ";
    print_vector(nums1);
    std::cout << ", Output: " << (xorGame(nums1) ? "true" : "false") << std::endl;

    // Test Case 2: Example 2, Expected: true
    std::vector<int> nums2 = {0, 1};
    std::cout << "Input: ";
    print_vector(nums2);
    std::cout << ", Output: " << (xorGame(nums2) ? "true" : "false") << std::endl;

    // Test Case 3: Example 3, Expected: true
    std::vector<int> nums3 = {1, 2, 3};
    std::cout << "Input: ";
    print_vector(nums3);
    std::cout << ", Output: " << (xorGame(nums3) ? "true" : "false") << std::endl;

    // Test Case 4: N even, XOR non-zero. Expected: true
    std::vector<int> nums4 = {1, 2, 3, 6};
    std::cout << "Input: ";
    print_vector(nums4);
    std::cout << ", Output: " << (xorGame(nums4) ? "true" : "false") << std::endl;

    // Test Case 5: N odd, XOR non-zero. Expected: false
    std::vector<int> nums5 = {4, 5, 2};
    std::cout << "Input: ";
    print_vector(nums5);
    std::cout << ", Output: " << (xorGame(nums5) ? "true" : "false") << std::endl;

    return 0;
}