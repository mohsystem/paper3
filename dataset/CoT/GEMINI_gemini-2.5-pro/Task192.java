public class Task192 {

    /**
     * Determines if Alice wins the XOR game.
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
     * @param nums The array of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    public boolean xorGame(int[] nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }
        return xorSum == 0 || nums.length % 2 == 0;
    }

    public static void main(String[] args) {
        Task192 game = new Task192();

        // Test Case 1
        int[] nums1 = {1, 1, 2};
        System.out.println("Test Case 1: nums = [1, 1, 2]");
        System.out.println("Output: " + game.xorGame(nums1)); // Expected: false

        // Test Case 2
        int[] nums2 = {0, 1};
        System.out.println("\nTest Case 2: nums = [0, 1]");
        System.out.println("Output: " + game.xorGame(nums2)); // Expected: true

        // Test Case 3
        int[] nums3 = {1, 2, 3};
        System.out.println("\nTest Case 3: nums = [1, 2, 3]");
        System.out.println("Output: " + game.xorGame(nums3)); // Expected: true

        // Test Case 4
        int[] nums4 = {1, 1};
        System.out.println("\nTest Case 4: nums = [1, 1]");
        System.out.println("Output: " + game.xorGame(nums4)); // Expected: true

        // Test Case 5
        int[] nums5 = {5, 4, 3, 2, 1};
        System.out.println("\nTest Case 5: nums = [5, 4, 3, 2, 1]");
        System.out.println("Output: " + game.xorGame(nums5)); // Expected: false
    }
}