import java.util.List;
import java.util.Arrays;

public class Task18 {

    /**
     * Given an array of ones and zeroes, convert the equivalent binary value to an integer.
     * @param binary A list of integers (0s and 1s).
     * @return The integer representation of the binary value.
     */
    public static int binaryArrayToInteger(List<Integer> binary) {
        if (binary == null) {
            return 0;
        }
        int number = 0;
        for (int bit : binary) {
            // Left shift the current number by 1 and add the new bit.
            number = (number << 1) | bit;
        }
        return number;
    }

    public static void main(String[] args) {
        // Test Case 1
        List<Integer> test1 = Arrays.asList(0, 0, 0, 1);
        System.out.println("Testing: [0, 0, 0, 1] ==> " + binaryArrayToInteger(test1));

        // Test Case 2
        List<Integer> test2 = Arrays.asList(0, 0, 1, 0);
        System.out.println("Testing: [0, 0, 1, 0] ==> " + binaryArrayToInteger(test2));

        // Test Case 3
        List<Integer> test3 = Arrays.asList(0, 1, 0, 1);
        System.out.println("Testing: [0, 1, 0, 1] ==> " + binaryArrayToInteger(test3));

        // Test Case 4
        List<Integer> test4 = Arrays.asList(1, 0, 0, 1);
        System.out.println("Testing: [1, 0, 0, 1] ==> " + binaryArrayToInteger(test4));

        // Test Case 5
        List<Integer> test5 = Arrays.asList(1, 1, 1, 1);
        System.out.println("Testing: [1, 1, 1, 1] ==> " + binaryArrayToInteger(test5));
    }
}