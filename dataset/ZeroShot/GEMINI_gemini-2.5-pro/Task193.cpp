#include <iostream>
#include <vector>
#include <numeric>

class Task193 {
public:
    /**
     * @brief Determines if Alice wins the Chalkboard XOR Game.
     * 
     * @param nums The vector of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    bool xorGame(const std::vector<int>& nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }

        // Alice wins if she starts with an XOR sum of 0,
        // or if the number of elements is even.
        // If n is even and xorSum != 0, Alice can always make a move
        // such that the remaining xorSum is not 0. Bob will be left
        // with an odd number of elements. The game continues until Bob
        // is forced to take the last element, making the xorSum 0 and losing.
        return xorSum == 0 || nums.size() % 2 == 0;
    }
};

void run_test_case(Task193& game, const std::string& name, const std::vector<int>& nums, bool expected) {
    bool result = game.xorGame(nums);
    std::cout << name << ": nums = [";
    for(size_t i = 0; i < nums.size(); ++i) {
        std::cout << nums[i] << (i == nums.size() - 1 ? "" : ", ");
    }
    std::cout << "], Expected: " << std::boolalpha << expected
              << ", Got: " << std::boolalpha << result << std::endl;
}

int main() {
    Task193 game;

    // Test Case 1
    std::vector<int> nums1 = {1, 1, 2};
    run_test_case(game, "Test Case 1", nums1, false);

    // Test Case 2
    std::vector<int> nums2 = {0, 1};
    run_test_case(game, "Test Case 2", nums2, true);

    // Test Case 3
    std::vector<int> nums3 = {1, 2, 3};
    run_test_case(game, "Test Case 3", nums3, true);

    // Test Case 4: n is even, xorSum != 0
    std::vector<int> nums4 = {1, 2, 3, 4}; // xorSum = 4
    run_test_case(game, "Test Case 4", nums4, true);

    // Test Case 5: n is odd, xorSum != 0
    std::vector<int> nums5 = {1, 2, 4}; // xorSum = 7
    run_test_case(game, "Test Case 5", nums5, false);

    return 0;
}