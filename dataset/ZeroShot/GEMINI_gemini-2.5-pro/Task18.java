import java.math.BigInteger;
import java.util.Arrays;
import java.util.List;

public class Task18 {

    /**
     * Converts a list of binary digits (0s and 1s) to its integer equivalent.
     * Uses BigInteger to prevent overflow, allowing for arrays of any length.
     *
     * @param binary A List of Integers, where each integer is either 0 or 1.
     * @return The BigInteger representation of the binary value.
     * @throws IllegalArgumentException if the list is null, or contains elements other than 0 or 1.
     */
    public static BigInteger binaryArrayToInteger(List<Integer> binary) {
        if (binary == null) {
            throw new IllegalArgumentException("Input list cannot be null.");
        }

        BigInteger result = BigInteger.ZERO;
        for (Integer bit : binary) {
            if (bit != 0 && bit != 1) {
                throw new IllegalArgumentException("Array must contain only 0s and 1s.");
            }
            // Equivalent to: result = result * 2 + bit
            result = result.shiftLeft(1).add(BigInteger.valueOf(bit));
        }
        return result;
    }

    public static void main(String[] args) {
        // 5 Test cases
        List<List<Integer>> testCases = Arrays.asList(
            Arrays.asList(0, 0, 0, 1), // ==> 1
            Arrays.asList(0, 0, 1, 0), // ==> 2
            Arrays.asList(0, 1, 0, 1), // ==> 5
            Arrays.asList(1, 0, 0, 1), // ==> 9
            Arrays.asList(1, 1, 1, 1)  // ==> 15
        );

        for (int i = 0; i < testCases.size(); i++) {
            List<Integer> testCase = testCases.get(i);
            try {
                BigInteger result = binaryArrayToInteger(testCase);
                System.out.println("Testing: " + testCase + " ==> " + result);
            } catch (IllegalArgumentException e) {
                System.out.println("Error processing " + testCase + ": " + e.getMessage());
            }
        }
    }
}