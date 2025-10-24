import java.util.Arrays;

public class Task9 {

    /**
     * Given a list of integers, determine whether the sum of its elements is odd or even.
     * @param array The input array of integers.
     * @return "even" or "odd".
     */
    public static String oddOrEven(int[] array) {
        // The sum of an empty array is 0, which is even.
        // We can use Java Streams for a concise solution.
        int sum = Arrays.stream(array).sum();
        
        if (sum % 2 == 0) {
            return "even";
        } else {
            return "odd";
        }
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {0};
        System.out.println("Input: " + Arrays.toString(arr1) + ", Output: " + oddOrEven(arr1));

        // Test Case 2
        int[] arr2 = {0, 1, 4};
        System.out.println("Input: " + Arrays.toString(arr2) + ", Output: " + oddOrEven(arr2));

        // Test Case 3
        int[] arr3 = {0, -1, -5};
        System.out.println("Input: " + Arrays.toString(arr3) + ", Output: " + oddOrEven(arr3));

        // Test Case 4: Empty array
        int[] arr4 = {};
        System.out.println("Input: " + Arrays.toString(arr4) + ", Output: " + oddOrEven(arr4));
        
        // Test Case 5
        int[] arr5 = {1, 2, 3, 4, 5};
        System.out.println("Input: " + Arrays.toString(arr5) + ", Output: " + oddOrEven(arr5));
    }
}