import java.util.Random;

public class Lse24 {

    /**
     * Generates two random integers within a specified range, calculates their difference,
     * and prints the results.
     *
     * @param min The minimum value of the random number range (inclusive).
     * @param max The maximum value of the random number range (inclusive).
     */
    public static void generateAndCalculateDifference(int min, int max) {
        if (min > max) {
            System.err.println("Error: min value cannot be greater than max value.");
            return;
        }

        Random random = new Random();

        // Generate a random integer between min and max.
        // nextInt(bound) generates a number between 0 (inclusive) and bound (exclusive).
        // To get a number in [min, max], we need a bound of (max - min + 1) and then add min.
        int num1 = random.nextInt(max - min + 1) + min;

        // Generate another random integer between min and max.
        int num2 = random.nextInt(max - min + 1) + min;

        // Calculate the difference between the two integers.
        int difference = num1 - num2;

        // Print the results.
        System.out.println("First random number: " + num1);
        System.out.println("Second random number: " + num2);
        System.out.println("Difference (" + num1 + " - " + num2 + "): " + difference);
    }

    public static void main(String[] args) {
        final int MIN_VAL = 10;
        final int MAX_VAL = 1000;

        System.out.println("Running 5 test cases...");
        for (int i = 0; i < 5; i++) {
            System.out.println("\nTest Case " + (i + 1) + ":");
            generateAndCalculateDifference(MIN_VAL, MAX_VAL);
        }
    }
}