import java.util.Arrays;

public class Task193 {

    public boolean xorGame(int[] nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }
        return xorSum == 0 || nums.length % 2 == 0;
    }

    public static void main(String[] args) {
        Task193 solver = new Task193();

        // Test Case 1
        int[] nums1 = {1, 1, 2};
        System.out.println("Test Case 1: nums = " + Arrays.toString(nums1) + ", Result: " + solver.xorGame(nums1));

        // Test Case 2
        int[] nums2 = {0, 1};
        System.out.println("Test Case 2: nums = " + Arrays.toString(nums2) + ", Result: " + solver.xorGame(nums2));
        
        // Test Case 3
        int[] nums3 = {1, 2, 3};
        System.out.println("Test Case 3: nums = " + Arrays.toString(nums3) + ", Result: " + solver.xorGame(nums3));
        
        // Test Case 4
        int[] nums4 = {1, 1};
        System.out.println("Test Case 4: nums = " + Arrays.toString(nums4) + ", Result: " + solver.xorGame(nums4));

        // Test Case 5
        int[] nums5 = {1, 2, 3, 4, 5, 6, 7};
        System.out.println("Test Case 5: nums = " + Arrays.toString(nums5) + ", Result: " + solver.xorGame(nums5));
    }
}