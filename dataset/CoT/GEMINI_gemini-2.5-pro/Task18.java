import java.util.List;
import java.util.Arrays;

public class Task18 {

    /**
     * Given a list of ones and zeroes, convert the equivalent binary value to an integer.
     *
     * @param binary A list of integers (0s and 1s).
     * @return The integer representation of the binary value.
     */
    public static int binaryArrayToNumber(List<Integer> binary) {
        int number = 0;
        for (int bit : binary) {
            // Left shift the current number by 1 and add the new bit using bitwise OR.
            // This is equivalent to number = number * 2 + bit;
            number = (number << 1) | bit;
        }
        return number;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(binaryArrayToNumber(Arrays.asList(0, 0, 0, 1))); // Expected: 1
        // Test Case 2
        System.out.println(binaryArrayToNumber(Arrays.asList(0, 0, 1, 0))); // Expected: 2
        // Test Case 3
        System.out.println(binaryArrayToNumber(Arrays.asList(0, 1, 0, 1))); // Expected: 5
        // Test Case 4
        System.out.println(binaryArrayToNumber(Arrays.asList(1, 0, 0, 1))); // Expected: 9
        // Test Case 5
        System.out.println(binaryArrayToNumber(Arrays.asList(1, 1, 1, 1))); // Expected: 15
    }
}