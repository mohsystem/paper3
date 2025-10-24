public class Task192 {

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
        Task192 solution = new Task192();

        // Test Case 1
        int[] nums1 = {1, 1, 2};
        System.out.println(solution.xorGame(nums1)); // Expected: false

        // Test Case 2
        int[] nums2 = {0, 1};
        System.out.println(solution.xorGame(nums2)); // Expected: true

        // Test Case 3
        int[] nums3 = {1, 2, 3};
        System.out.println(solution.xorGame(nums3)); // Expected: true

        // Test Case 4
        int[] nums4 = {6, 6, 6, 6};
        System.out.println(solution.xorGame(nums4)); // Expected: true

        // Test Case 5
        int[] nums5 = {5, 3, 2, 1};
        System.out.println(solution.xorGame(nums5)); // Expected: true
    }
}