import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;

public class Task18 {

    /**
     * Converts a list of binary digits (0s and 1s) to its integer equivalent.
     * @param binary A list of integers, where each integer is either 0 or 1.
     * @return The decimal integer value of the binary representation.
     */
    public static int binaryArrayToNumber(List<Integer> binary) {
        int number = 0;
        for (int bit : binary) {
            // Left shift the current number by 1 and add the new bit.
            // This is equivalent to number = number * 2 + bit;
            number = (number << 1) | bit;
        }
        return number;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(binaryArrayToNumber(new ArrayList<>(Arrays.asList(0, 0, 0, 1)))); // Expected: 1
        System.out.println(binaryArrayToNumber(new ArrayList<>(Arrays.asList(0, 1, 0, 1)))); // Expected: 5
        System.out.println(binaryArrayToNumber(new ArrayList<>(Arrays.asList(1, 0, 0, 1)))); // Expected: 9
        System.out.println(binaryArrayToNumber(new ArrayList<>(Arrays.asList(1, 1, 1, 1)))); // Expected: 15
        System.out.println(binaryArrayToNumber(new ArrayList<>(Arrays.asList(1, 0, 1, 1)))); // Expected: 11
    }
}