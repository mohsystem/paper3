import java.util.Arrays;
import java.util.stream.IntStream;

public class Task9 {

    /**
     * Determines if the sum of elements in an integer array is "odd" or "even".
     * An empty array is treated as [0].
     *
     * @param array The input array of integers.
     * @return "even" if the sum is even, "odd" otherwise.
     */
    public static String oddOrEven(int[] array) {
        // A more robust way than summing to avoid potential overflow is to check the parity.
        // The sum of integers is odd if and only if the number of odd integers is odd.
        // However, using a long for the sum is sufficient for most practical cases.
        // Java 8 Stream API provides a concise way to sum.
        long sum = Arrays.stream(array).asLongStream().sum();
        
        if (sum % 2 == 0) {
            return "even";
        } else {
            return "odd";
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Single zero
        int[] test1 = {0};
        System.out.println("Input: " + Arrays.toString(test1) + " -> Output: \"" + oddOrEven(test1) + "\"");

        // Test Case 2: Mix of numbers, odd sum
        int[] test2 = {0, 1, 4};
        System.out.println("Input: " + Arrays.toString(test2) + " -> Output: \"" + oddOrEven(test2) + "\"");

        // Test Case 3: Negative numbers, even sum
        int[] test3 = {0, -1, -5};
        System.out.println("Input: " + Arrays.toString(test3) + " -> Output: \"" + oddOrEven(test3) + "\"");
        
        // Test Case 4: Empty array
        int[] test4 = {};
        System.out.println("Input: " + Arrays.toString(test4) + " -> Output: \"" + oddOrEven(test4) + "\"");

        // Test Case 5: Sequence of numbers, odd sum
        int[] test5 = {1, 2, 3, 4, 5};
        System.out.println("Input: " + Arrays.toString(test5) + " -> Output: \"" + oddOrEven(test5) + "\"");
    }
}