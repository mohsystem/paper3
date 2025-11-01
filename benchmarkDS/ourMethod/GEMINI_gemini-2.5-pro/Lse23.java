import java.security.SecureRandom;

public class Lse23 {

    /**
     * Generates three random integers larger than 1000, calculates their sum,
     * and prints the results in a specific format.
     */
    public static void generateAndPrintRandomSum() {
        // Use SecureRandom for cryptographically strong random numbers.
        SecureRandom random = new SecureRandom();
        
        // Generate three random integers larger than 1000.
        // We'll generate numbers in the range [1001, 9999].
        // The size of the range is 9999 - 1001 + 1 = 8999.
        // random.nextInt(bound) generates from 0 to bound-1.
        int a = random.nextInt(8999) + 1001;
        int b = random.nextInt(8999) + 1001;
        int c = random.nextInt(8999) + 1001;

        // Add the three integers.
        // Using long for the sum is a good practice to prevent potential overflow,
        // although an int would suffice for the chosen range.
        long sum = (long)a + b + c;

        // Print the results in the specified format.
        System.out.printf("a : %d b : %d c : %d sum: %d sum2: %d%n", a, b, c, sum, sum);
    }

    /**
     * Main method to run the test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // Run 5 test cases as requested.
        for (int i = 0; i < 5; i++) {
            generateAndPrintRandomSum();
        }
    }
}