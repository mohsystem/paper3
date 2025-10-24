#include <iostream>
#include <vector>
#include <numeric>
#include <functional>

/**
 * Determines if Alice wins the XOR game.
 *
 * The game has two key conditions:
 * 1. If the bitwise XOR of all numbers on the board is 0 at the start of a player's turn, that player wins.
 * 2. A player loses if their move causes the XOR sum of the remaining numbers to become 0.
 *
 * Let's analyze the game based on these rules:
 *
 * Case 1: The initial XOR sum of all numbers is 0.
 * Alice starts her turn, and the XOR sum is 0. According to rule 1, she wins immediately.
 *
 * Case 2: The initial XOR sum is not 0.
 * The game must continue. A player loses if they are forced to make a move that results in an XOR sum of 0.
 * Let the current XOR sum be `X` and the numbers on the board be `n_1, n_2, ..., n_k`.
 * A player loses if for every number `n_i` they could remove, the new XOR sum `X ^ n_i` is 0. This implies `X = n_i` for all `i`.
 *
 * Now, consider the number of elements `k` on the board:
 * - If `k` is even: If all `n_i` were equal to `X`, their total XOR sum would be 0 (since an even number of identical values XORed together is 0). This contradicts the assumption that the current XOR sum `X` is not 0. Therefore, if `k` is even, there must be at least one number `n_j` such that `n_j != X`. A player can always choose to remove such an `n_j`, ensuring the new XOR sum `X ^ n_j` is not 0. Thus, a player whose turn starts with an even number of elements (and non-zero XOR sum) can never be forced to lose on that turn.
 *
 * - If `k` is odd: It's possible for a player to be in a losing state. If all `k` numbers are identical and equal to `X`, their XOR sum is `X`. The player must remove one `X`, leaving `k-1` (an even number) of `X`'s, whose XOR sum is 0. The player loses.
 *
 * Game progression when initial XOR sum is not 0:
 * - If the starting number of elements `N` is even: Alice starts. Since `N` is even, she can always make a move that doesn't lose. She leaves `N-1` (odd) numbers for Bob. Bob's turns will always start with an odd number of elements, while Alice's subsequent turns will start with an even number of elements. Since Bob is the one who always faces the "risky" odd-sized boards, and Alice always faces the "safe" even-sized boards, Bob will eventually be forced into a losing position (e.g., facing a single number which he must remove). Therefore, Alice wins.
 *
 * - If the starting number of elements `N` is odd: Alice starts. She is in the "risky" position. Bob will always face an even number of elements, so he can never be forced to lose on his turn. Eventually, Alice will be forced into a losing position. Therefore, Alice loses.
 *
 * Summary:
 * - Alice wins if the initial XOR sum is 0.
 * - Alice wins if the initial XOR sum is not 0 AND the number of elements is even.
 *
 * @param nums The vector of integers on the chalkboard.
 * @return true if Alice wins, false otherwise.
 */
bool xorGame(const std::vector<int>& nums) {
    int xorSum = 0;
    for (int num : nums) {
        xorSum ^= num;
    }
    
    // Alice wins if the initial XOR sum is 0 (she wins on her first turn).
    // Or, if the initial XOR sum is not 0 but there's an even number of elements.
    // In this case, a losing move is impossible on any turn with an even number of elements,
    // so Alice (who always plays on even counts: N, N-2, ...) can't be forced to lose.
    return xorSum == 0 || nums.size() % 2 == 0;
}

int main() {
    // Test Case 1: N=3 (odd), XOR=2. Alice loses.
    std::vector<int> nums1 = {1, 1, 2};
    std::cout << "Input: [1, 1, 2], Output: " << std::boolalpha << xorGame(nums1) << std::endl;

    // Test Case 2: N=2 (even), XOR=1. Alice wins.
    std::vector<int> nums2 = {0, 1};
    std::cout << "Input: [0, 1], Output: " << std::boolalpha << xorGame(nums2) << std::endl;

    // Test Case 3: N=3 (odd), XOR=0. Alice wins immediately.
    std::vector<int> nums3 = {1, 2, 3};
    std::cout << "Input: [1, 2, 3], Output: " << std::boolalpha << xorGame(nums3) << std::endl;

    // Test Case 4: N=2 (even), XOR=0. Alice wins immediately.
    std::vector<int> nums4 = {1, 1};
    std::cout << "Input: [1, 1], Output: " << std::boolalpha << xorGame(nums4) << std::endl;

    // Test Case 5: N=4 (even), XOR=4. Alice wins.
    std::vector<int> nums5 = {1, 2, 3, 4};
    std::cout << "Input: [1, 2, 3, 4], Output: " << std::boolalpha << xorGame(nums5) << std::endl;

    return 0;
}