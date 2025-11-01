import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

public class Lse23 {

    /**
     * Generates three random integers larger than 1000, calculates their sum,
     * and prints the results in a specific format.
     */
    public static void generateAndPrintRandomSum() {
        // Using ThreadLocalRandom is efficient in multithreaded contexts and a good practice.
        // We generate numbers in the range [1001, 9999] to ensure they are > 1000.
        // The upper bound in nextInt is exclusive, so we use 10000.
        int a = ThreadLocalRandom.current().nextInt(1001, 10000);
        int b = ThreadLocalRandom.current().nextInt(1001, 10000);
        int c = ThreadLocalRandom.current().nextInt(1001, 10000);

        // Add the three integers. The sum will not overflow a 32-bit integer
        // as the maximum sum is 9999 * 3 = 29997.
        int sum = a + b + c;

        // Print the result in the required format.
        System.out.printf("a : %d b : %d c : %d sum: %d sum2: %d\n", a, b, c, sum, sum);
    }

    public static void main(String[] args) {
        // Run 5 test cases as requested.
        System.out.println("Running Java Test Cases:");
        for (int i = 0; i < 5; i++) {
            generateAndPrintRandomSum();
        }
    }
}