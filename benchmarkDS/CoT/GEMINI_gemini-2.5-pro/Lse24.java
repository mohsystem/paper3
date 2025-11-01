import java.util.concurrent.ThreadLocalRandom;

public class Lse24 {

    /**
     * Generates two random integers between 10 and 1000, calculates their
     * difference, and prints the results.
     */
    public static void generateAndCalculateDifference() {
        // Generate the first random integer between 10 and 1000 (inclusive).
        int num1 = ThreadLocalRandom.current().nextInt(10, 1001);

        // Generate the second random integer between 10 and 1000 (inclusive).
        int num2 = ThreadLocalRandom.current().nextInt(10, 1001);

        // Calculate the difference between the two integers.
        int difference = num1 - num2;

        // Print the results.
        System.out.println("First number: " + num1);
        System.out.println("Second number: " + num2);
        System.out.println("Difference (First - Second): " + difference);
    }

    public static void main(String[] args) {
        System.out.println("Running 5 test cases:");
        for (int i = 0; i < 5; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            generateAndCalculateDifference();
        }
    }
}