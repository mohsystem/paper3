#include <iostream>
#include <vector>
#include <numeric>
#include <functional>

class Task192 {
public:
    /**
     * @brief Determines if Alice wins the XOR game.
     *
     * The logic is based on a game theory analysis:
     * A player wins if they start their turn and the XOR sum of all numbers is 0.
     * A player loses if any move they make results in an XOR sum of 0.
     * An optimal player will always make a move to a state that is a losing position for the opponent, if possible.
     *
     * Let's analyze the game based on the number of elements N.
     * Case 1: The initial XOR sum of all numbers is 0.
     * Alice starts with an XOR sum of 0, so she wins immediately.
     *
     * Case 2: The initial XOR sum is not 0.
     *   - If N is even: Alice can always choose a number `x` to remove such that the new XOR sum is not 0.
     *     This leaves Bob with N-1 (an odd number) elements and a non-zero XOR sum.
     *     A game state with an odd number of elements and a non-zero XOR sum is a losing position.
     *     Therefore, Alice can force a win.
     *   - If N is odd: Any number Alice removes will leave N-1 (an even number) elements for Bob.
     *     A game state with an even number of elements is always a winning position for the player whose turn it is (Bob in this case).
     *     Therefore, Alice cannot win.
     *
     * Combining these, Alice wins if the initial XOR sum is 0, OR if the number of elements is even.
     *
     * @param nums The vector of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    bool xorGame(const std::vector<int>& nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }
        // Alternatively using std::accumulate:
        // int xorSum = std::accumulate(nums.begin(), nums.end(), 0, std::bit_xor<int>());
        
        return xorSum == 0 || nums.size() % 2 == 0;
    }
};

void print_vector(const std::vector<int>& nums) {
    std::cout << "[";
    for (size_t i = 0; i < nums.size(); ++i) {
        std::cout << nums[i] << (i == nums.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    Task192 game;

    // Test Case 1
    std::vector<int> nums1 = {1, 1, 2};
    std::cout << "Test Case 1: nums = ";
    print_vector(nums1);
    std::cout << std::endl;
    std::cout << "Output: " << (game.xorGame(nums1) ? "true" : "false") << std::endl; // Expected: false

    // Test Case 2
    std::vector<int> nums2 = {0, 1};
    std::cout << "\nTest Case 2: nums = ";
    print_vector(nums2);
    std::cout << std::endl;
    std::cout << "Output: " << (game.xorGame(nums2) ? "true" : "false") << std::endl; // Expected: true

    // Test Case 3
    std::vector<int> nums3 = {1, 2, 3};
    std::cout << "\nTest Case 3: nums = ";
    print_vector(nums3);
    std::cout << std::endl;
    std::cout << "Output: " << (game.xorGame(nums3) ? "true" : "false") << std::endl; // Expected: true

    // Test Case 4
    std::vector<int> nums4 = {1, 1};
    std::cout << "\nTest Case 4: nums = ";
    print_vector(nums4);
    std::cout << std::endl;
    std::cout << "Output: " << (game.xorGame(nums4) ? "true" : "false") << std::endl; // Expected: true

    // Test Case 5
    std::vector<int> nums5 = {5, 4, 3, 2, 1};
    std::cout << "\nTest Case 5: nums = ";
    print_vector(nums5);
    std::cout << std::endl;
    std::cout << "Output: " << (game.xorGame(nums5) ? "true" : "false") << std::endl; // Expected: false

    return 0;
}