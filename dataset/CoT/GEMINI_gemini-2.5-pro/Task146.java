public class Task146 {

    /**
     * Finds the missing number in an array containing n-1 unique integers
     * from the range [1, n].
     *
     * @param nums The input array of unique integers.
     * @return The missing integer.
     */
    public static int findMissingNumber(int[] nums) {
        if (nums == null) {
            // Or handle as an error, depending on requirements.
            // For an empty range (n=0), this is ambiguous.
            // Assuming valid inputs as per prompt.
            return 0;
        }

        // 'n' is the expected size of the complete sequence, which is one more than the array length.
        int n = nums.length + 1;

        // Use long to prevent potential integer overflow for large n.
        // Formula for the sum of the first n natural numbers: n * (n + 1) / 2
        long expectedSum = (long)n * (n + 1) / 2;

        long actualSum = 0;
        for (int num : nums) {
            actualSum += num;
        }

        // The difference is the missing number.
        return (int)(expectedSum - actualSum);
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {1, 2, 4, 5};
        System.out.println("Test Case 1: Input: [1, 2, 4, 5], Missing Number: " + findMissingNumber(arr1));

        // Test Case 2
        int[] arr2 = {2, 3, 1, 5};
        System.out.println("Test Case 2: Input: [2, 3, 1, 5], Missing Number: " + findMissingNumber(arr2));

        // Test Case 3
        int[] arr3 = {1};
        System.out.println("Test Case 3: Input: [1], Missing Number: " + findMissingNumber(arr3));

        // Test Case 4
        int[] arr4 = {2};
        System.out.println("Test Case 4: Input: [2], Missing Number: " + findMissingNumber(arr4));
        
        // Test Case 5 (Edge case: empty array)
        int[] arr5 = {};
        System.out.println("Test Case 5: Input: [], Missing Number: " + findMissingNumber(arr5));
    }
}