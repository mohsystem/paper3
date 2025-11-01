import java.security.SecureRandom;

public class Lse23 {

    /**
     * Generates three cryptographically secure random integers greater than 1000,
     * prints them, calculates their sum safely to prevent overflow, and prints the sum.
     */
    public static void generateAndPrintSum() {
        // Use SecureRandom for cryptographically strong random numbers.
        SecureRandom random = new SecureRandom();

        // Generate three integers, each greater than 1000.
        // We generate a number in [0, 29000) and add 1001 to get a range of [1001, 30000].
        int a = random.nextInt(29000) + 1001;
        int b = random.nextInt(29000) + 1001;
        int c = random.nextInt(29000) + 1001;

        // Use 'long' for the sum to prevent integer overflow, which is a security risk.
        // (e.g., Integer.MAX_VALUE + 1 wraps around to a negative number).
        // Casting the first operand to long promotes the entire expression to long.
        long sum = (long) a + b + c;

        // Print the numbers and their sum in the specified format.
        System.out.printf("a : %d b : %d c : %d sum: %d sum2: %d%n", a, b, c, sum, sum);
    }

    public static void main(String[] args) {
        System.out.println("Java execution:");
        // Run 5 test cases as requested.
        for (int i = 0; i < 5; i++) {
            generateAndPrintSum();
        }
    }
}