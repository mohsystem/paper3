import java.util.Arrays;

/**
 * The Task146 class provides a method to find a missing number in an array.
 */
public class Task146 {

    /**
     * Finds the missing number in an array containing unique integers from 1 to n.
     * The array is guaranteed to have n-1 elements.
     *
     * @param nums An array of unique integers from 1 to n with one number missing.
     *             It must not be null, but can be empty.
     * @return The missing number.
     */
    public static int findMissingNumber(int[] nums) {
        // According to the problem description, the input is an array of unique
        // integers from 1 to n, with one number missing. This means the array
        // will have a size of n-1.

        // If the array is empty, it means n=1, the full sequence is {1},
        // and the missing number is 1. A null check is good practice.
        if (nums == null || nums.length == 0) {
            return 1;
        }

        // The full sequence should contain n numbers. The given array has n-1.
        // So, n is the array length + 1.
        int n = nums.length + 1;

        // To avoid potential integer overflow when n is large, we use 'long'
        // for the sum calculation. The formula for the sum of the first n
        // natural numbers is n * (n + 1) / 2.
        long expectedSum = (long) n * (n + 1) / 2;

        long actualSum = 0;
        // The for-each loop is a safe way to iterate without boundary errors.
        for (int num : nums) {
            actualSum += num;
        }

        // The difference between the expected sum and the actual sum is the missing number.
        // The result will fit in an int as it's one of the numbers from 1 to n.
        return (int) (expectedSum - actualSum);
    }

    /**
     * Main method with test cases.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // 5 test cases
        int[] test1 = {1, 2, 4, 5}; // n=5, missing 3
        System.out.println("Test 1: Array=" + Arrays.toString(test1) + ", Missing=" + findMissingNumber(test1));

        int[] test2 = {2, 3, 1, 5}; // n=5, missing 4
        System.out.println("Test 2: Array=" + Arrays.toString(test2) + ", Missing=" + findMissingNumber(test2));

        int[] test3 = {1}; // n=2, missing 2
        System.out.println("Test 3: Array=" + Arrays.toString(test3) + ", Missing=" + findMissingNumber(test3));

        int[] test4 = {}; // n=1, missing 1
        System.out.println("Test 4: Array=" + Arrays.toString(test4) + ", Missing=" + findMissingNumber(test4));

        int[] test5 = {1, 2, 3, 4, 5, 6, 7, 9, 10}; // n=10, missing 8
        System.out.println("Test 5: Array=" + Arrays.toString(test5) + ", Missing=" + findMissingNumber(test5));
    }
}