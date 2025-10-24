#include <iostream>
#include <vector>
#include <numeric>
#include <functional>

class Task192 {
public:
    /**
     * Determines if Alice wins the XOR game.
     *
     * Alice wins if:
     * 1. The bitwise XOR of all numbers on the board is 0 at the start of her turn.
     * 2. The number of elements on the board is even. If the XOR sum is not 0,
     *    she can always make a move that doesn't result in an XOR sum of 0,
     *    passing the turn to Bob. Since Alice always faces an even number of
     *    elements and Bob an odd number, Bob is the one who might be forced
     *    into a losing position.
     *
     * @param nums The vector of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    bool xorGame(const std::vector<int>& nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }
        return xorSum == 0 || nums.size() % 2 == 0;
    }
};

int main() {
    Task192 solution;
    std::cout << std::boolalpha;

    // Test Case 1
    std::vector<int> nums1 = {1, 1, 2};
    std::cout << solution.xorGame(nums1) << std::endl; // Expected: false

    // Test Case 2
    std::vector<int> nums2 = {0, 1};
    std::cout << solution.xorGame(nums2) << std::endl; // Expected: true

    // Test Case 3
    std::vector<int> nums3 = {1, 2, 3};
    std::cout << solution.xorGame(nums3) << std::endl; // Expected: true

    // Test Case 4
    std::vector<int> nums4 = {6, 6, 6, 6};
    std::cout << solution.xorGame(nums4) << std::endl; // Expected: true

    // Test Case 5
    std::vector<int> nums5 = {5, 3, 2, 1};
    std::cout << solution.xorGame(nums5) << std::endl; // Expected: true

    return 0;
}