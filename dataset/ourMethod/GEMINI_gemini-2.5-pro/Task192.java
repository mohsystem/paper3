public class Task192 {

    /**
     * Determines if Alice wins the XOR game.
     * Alice wins if:
     * 1. The bitwise XOR of all elements is 0 at the start of her turn.
     * 2. The number of elements is even, and the initial XOR is not 0. In this case,
     *    she can always make a move that doesn't result in a losing state (XOR sum of 0),
     *    and since the total number of turns is even, Bob will be the one to make the last
     *    move, which will make the XOR sum 0, causing Bob to lose.
     * If the number of elements is odd and the initial XOR is not 0, Alice will eventually
     * be the one to make the last move, causing her to lose.
     *
     * @param nums An array of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    public boolean xorGame(int[] nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }

        // Alice wins if the initial XOR sum is 0 (she wins immediately)
        // or if the number of elements is even.
        return xorSum == 0 || nums.length % 2 == 0;
    }

    public static void main(String[] args) {
        Task192 solution = new Task192();

        // Test Case 1: Example 1, Expected: false
        int[] nums1 = {1, 1, 2};
        System.out.println("Input: [1, 1, 2], Output: " + solution.xorGame(nums1));

        // Test Case 2: Example 2, Expected: true
        int[] nums2 = {0, 1};
        System.out.println("Input: [0, 1], Output: " + solution.xorGame(nums2));

        // Test Case 3: Example 3, Expected: true
        int[] nums3 = {1, 2, 3};
        System.out.println("Input: [1, 2, 3], Output: " + solution.xorGame(nums3));

        // Test Case 4: N even, XOR non-zero. Expected: true
        int[] nums4 = {1, 2, 3, 6};
        System.out.println("Input: [1, 2, 3, 6], Output: " + solution.xorGame(nums4));

        // Test Case 5: N odd, XOR non-zero. Expected: false
        int[] nums5 = {4, 5, 2};
        System.out.println("Input: [4, 5, 2], Output: " + solution.xorGame(nums5));
    }
}