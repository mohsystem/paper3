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
     * 1. The initial bitwise XOR of all numbers is 0. She wins on her first turn.
     * 2. The number of elements is even. In this case, if the initial XOR sum is not 0,
     *    Alice can always make a move that does not result in an XOR sum of 0.
     *    This is because for a player to be forced to lose, they must face a board
     *    where all elements are identical to the current XOR sum, which is only possible
     *    if the number of elements is odd. Since Alice always starts her turn with an
     *    even number of elements, she can never be forced to lose. The game will proceed
     *    to the end, and Bob will take the last turn, making him lose.
     *
     * @param nums The vector of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    bool xorGame(const std::vector<int>& nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }
        
        // Alternative using std::accumulate
        // int xorSum = std::accumulate(nums.begin(), nums.end(), 0, std::bit_xor<int>());

        if (xorSum == 0) {
            return true;
        }

        return nums.size() % 2 == 0;
    }
};

int main() {
    Task192 game;

    // Test Case 1: nums = [1,1,2], Output: false
    std::vector<int> nums1 = {1, 1, 2};
    std::cout << "Test Case 1 for [1, 1, 2]: " << std::boolalpha << game.xorGame(nums1) << std::endl;

    // Test Case 2: nums = [0,1], Output: true
    std::vector<int> nums2 = {0, 1};
    std::cout << "Test Case 2 for [0, 1]: " << std::boolalpha << game.xorGame(nums2) << std::endl;

    // Test Case 3: nums = [1,2,3], Output: true
    std::vector<int> nums3 = {1, 2, 3};
    std::cout << "Test Case 3 for [1, 2, 3]: " << std::boolalpha << game.xorGame(nums3) << std::endl;

    // Test Case 4: n is even, xorSum != 0 -> true
    std::vector<int> nums4 = {1, 2, 4, 8};
    std::cout << "Test Case 4 for [1, 2, 4, 8]: " << std::boolalpha << game.xorGame(nums4) << std::endl;

    // Test Case 5: n is odd, xorSum != 0 -> false
    std::vector<int> nums5 = {5, 5, 5};
    std::cout << "Test Case 5 for [5, 5, 5]: " << std::boolalpha << game.xorGame(nums5) << std::endl;

    return 0;
}