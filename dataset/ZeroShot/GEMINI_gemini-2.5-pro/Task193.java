import java.util.Arrays;

public class Task193 {
    /**
     * Determines if Alice wins the Chalkboard XOR Game.
     *
     * @param nums The array of integers on the chalkboard.
     * @return true if Alice wins, false otherwise.
     */
    public boolean xorGame(int[] nums) {
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
        return xorSum == 0 || nums.length % 2 == 0;
    }

    public static void main(String[] args) {
        Task193 game = new Task193();

        // Test Case 1
        int[] nums1 = {1, 1, 2};
        System.out.println("Test Case 1: nums = " + Arrays.toString(nums1) + ", Expected: false, Got: " + game.xorGame(nums1));

        // Test Case 2
        int[] nums2 = {0, 1};
        System.out.println("Test Case 2: nums = " + Arrays.toString(nums2) + ", Expected: true, Got: " + game.xorGame(nums2));

        // Test Case 3
        int[] nums3 = {1, 2, 3};
        System.out.println("Test Case 3: nums = " + Arrays.toString(nums3) + ", Expected: true, Got: " + game.xorGame(nums3));

        // Test Case 4: n is even, xorSum != 0
        int[] nums4 = {1, 2, 3, 4}; // xorSum = 4
        System.out.println("Test Case 4: nums = " + Arrays.toString(nums4) + ", Expected: true, Got: " + game.xorGame(nums4));

        // Test Case 5: n is odd, xorSum != 0
        int[] nums5 = {1, 2, 4}; // xorSum = 7
        System.out.println("Test Case 5: nums = " + Arrays.toString(nums5) + ", Expected: false, Got: " + game.xorGame(nums5));
    }
}