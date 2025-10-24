import java.util.Arrays;

class Task192 {
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
     * @param nums The array of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    public boolean xorGame(int[] nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }

        if (xorSum == 0) {
            return true;
        }

        return nums.length % 2 == 0;
    }

    public static void main(String[] args) {
        Task192 game = new Task192();

        // Test Case 1: nums = [1,1,2], Output: false
        int[] nums1 = {1, 1, 2};
        System.out.println("Test Case 1 for [1, 1, 2]: " + game.xorGame(nums1));

        // Test Case 2: nums = [0,1], Output: true
        int[] nums2 = {0, 1};
        System.out.println("Test Case 2 for [0, 1]: " + game.xorGame(nums2));

        // Test Case 3: nums = [1,2,3], Output: true
        int[] nums3 = {1, 2, 3};
        System.out.println("Test Case 3 for [1, 2, 3]: " + game.xorGame(nums3));

        // Test Case 4: n is even, xorSum != 0 -> true
        int[] nums4 = {1, 2, 4, 8};
        System.out.println("Test Case 4 for [1, 2, 4, 8]: " + game.xorGame(nums4));

        // Test Case 5: n is odd, xorSum != 0 -> false
        int[] nums5 = {5, 5, 5};
        System.out.println("Test Case 5 for [5, 5, 5]: " + game.xorGame(nums5));
    }
}