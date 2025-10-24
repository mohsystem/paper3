import java.util.Arrays;
import java.util.stream.IntStream;

public class Task9 {

    /**
     * Given a list of integers, determine whether the sum of its elements is odd or even.
     *
     * @param array The input array of integers.
     * @return "even" if the sum is even, "odd" otherwise.
     */
    public static String oddOrEven(int[] array) {
        // The sum of an empty array is 0, which is even.
        // This is equivalent to treating it as [0].
        long sum = 0;
        for (int number : array) {
            sum += number;
        }

        if (sum % 2 == 0) {
            return "even";
        } else {
            return "odd";
        }
    }

    /*
     * Alternative implementation using Java Streams.
     * public static String oddOrEven(int[] array) {
     *     return Arrays.stream(array).sum() % 2 == 0 ? "even" : "odd";
     * }
     */

    public static void main(String[] args) {
        // Test Case 1
        int[] test1 = {0};
        System.out.println("Input: " + Arrays.toString(test1));
        System.out.println("Output: " + oddOrEven(test1)); // Expected: even

        // Test Case 2
        int[] test2 = {0, 1, 4};
        System.out.println("Input: " + Arrays.toString(test2));
        System.out.println("Output: " + oddOrEven(test2)); // Expected: odd

        // Test Case 3
        int[] test3 = {0, -1, -5};
        System.out.println("Input: " + Arrays.toString(test3));
        System.out.println("Output: " + oddOrEven(test3)); // Expected: even

        // Test Case 4 (Empty array)
        int[] test4 = {};
        System.out.println("Input: " + Arrays.toString(test4));
        System.out.println("Output: " + oddOrEven(test4)); // Expected: even

        // Test Case 5
        int[] test5 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        System.out.println("Input: " + Arrays.toString(test5));
        System.out.println("Output: " + oddOrEven(test5)); // Expected: odd
    }
}