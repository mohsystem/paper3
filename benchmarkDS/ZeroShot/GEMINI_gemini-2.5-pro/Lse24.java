import java.security.SecureRandom;

public class Lse24 {

    // It's generally better to create one SecureRandom instance and reuse it.
    private static final SecureRandom secureRandom = new SecureRandom();

    /**
     * Generates two cryptographically secure random integers within a specified range,
     * calculates their absolute difference, and returns the result as a formatted string.
     *
     * @param min The minimum value of the range (inclusive).
     * @param max The maximum value of the range (inclusive).
     * @return A string describing the two numbers and their difference.
     * @throws IllegalArgumentException if min is greater than max.
     */
    public static String generateRandomsAndDifference(int min, int max) {
        if (min > max) {
            throw new IllegalArgumentException("Min value cannot be greater than Max value.");
        }
        // nextInt(bound) generates a value between 0 (inclusive) and bound (exclusive).
        // To get a value in [min, max], we need a range of size (max - min + 1).
        // So we generate a random number in [0, max - min] and add min to shift the range.
        int range = max - min + 1;
        int num1 = secureRandom.nextInt(range) + min;
        int num2 = secureRandom.nextInt(range) + min;
        int difference = Math.abs(num1 - num2);
        
        return String.format("Num1: %d, Num2: %d, Difference: %d", num1, num2, difference);
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases...");
        final int MIN_VAL = 10;
        final int MAX_VAL = 1000;

        for (int i = 0; i < 5; i++) {
            try {
                String result = generateRandomsAndDifference(MIN_VAL, MAX_VAL);
                System.out.println("Test Case " + (i + 1) + ": " + result);
            } catch (IllegalArgumentException e) {
                System.err.println("Error in test case " + (i + 1) + ": " + e.getMessage());
            }
        }
    }
}