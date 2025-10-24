import java.util.Arrays;

public class Task2 {

    /**
     * Formats an array of 10 integers (0-9) into a phone number string.
     *
     * @param numbers An array of 10 integers, where each integer is between 0 and 9.
     * @return A string representing the phone number, e.g., "(123) 456-7890".
     * @throws IllegalArgumentException if the input array is null, not of length 10,
     *                                  or contains numbers outside the 0-9 range.
     */
    public static String createPhoneNumber(int[] numbers) {
        if (numbers == null || numbers.length != 10) {
            throw new IllegalArgumentException("Input array must contain exactly 10 integers.");
        }

        for (int n : numbers) {
            if (n < 0 || n > 9) {
                throw new IllegalArgumentException("All integers in the array must be between 0 and 9.");
            }
        }

        return String.format("(%d%d%d) %d%d%d-%d%d%d%d",
                numbers[0], numbers[1], numbers[2],
                numbers[3], numbers[4], numbers[5],
                numbers[6], numbers[7], numbers[8], numbers[9]);
    }

    public static void main(String[] args) {
        // Test Case 1: Valid input
        int[] test1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
        System.out.println("Test 1: " + Arrays.toString(test1));
        System.out.println("Result: " + createPhoneNumber(test1));
        System.out.println();

        // Test Case 2: Another valid input
        int[] test2 = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        System.out.println("Test 2: " + Arrays.toString(test2));
        System.out.println("Result: " + createPhoneNumber(test2));
        System.out.println();

        // Test Case 3: Invalid length (too short)
        int[] test3 = {1, 2, 3};
        System.out.println("Test 3: " + Arrays.toString(test3));
        try {
            createPhoneNumber(test3);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 4: Invalid number (out of range)
        int[] test4 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        System.out.println("Test 4: " + Arrays.toString(test4));
        try {
            createPhoneNumber(test4);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 5: Null input
        int[] test5 = null;
        System.out.println("Test 5: null");
        try {
            createPhoneNumber(test5);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
    }
}