import java.util.Arrays;

public class Task9 {

    /**
     * Given a list of integers, determine whether the sum of its elements is odd or even.
     *
     * @param array An array of integers. If the array is empty, it's considered as [0].
     * @return A string, "even" or "odd".
     */
    public static String oddOrEven(int[] array) {
        // Use a long to store the sum to prevent potential integer overflow,
        // which is a security risk.
        long sum = 0;
        
        // The problem states to treat an empty array as [0].
        // Summing an empty array results in 0, and summing [0] also results in 0.
        // So, no special check for an empty array is needed.
        for (int num : array) {
            sum += num;
        }

        // Check if the final sum is even or odd using the modulo operator.
        if (sum % 2 == 0) {
            return "even";
        } else {
            return "odd";
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Standard even case
        int[] test1 = {0};
        System.out.println("Input: " + Arrays.toString(test1) + ", Output: " + oddOrEven(test1));

        // Test Case 2: Standard odd case
        int[] test2 = {0, 1, 4};
        System.out.println("Input: " + Arrays.toString(test2) + ", Output: " + oddOrEven(test2));

        // Test Case 3: Case with negative numbers
        int[] test3 = {0, -1, -5};
        System.out.println("Input: " + Arrays.toString(test3) + ", Output: " + oddOrEven(test3));

        // Test Case 4: Empty array
        int[] test4 = {};
        System.out.println("Input: " + Arrays.toString(test4) + ", Output: " + oddOrEven(test4));
        
        // Test Case 5: A longer list
        int[] test5 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        System.out.println("Input: " + Arrays.toString(test5) + ", Output: " + oddOrEven(test5));
    }
}